#!/usr/bin/env python3
"""Compile actual portable-generator output with Icarus -g2001; run independent benches."""
from pathlib import Path
import os
import argparse,hashlib,json,random,re,shutil,subprocess
ROOT=Path(__file__).resolve().parents[1]
WORK=Path(os.environ.get('ECEN21_M8_WORK',str(ROOT/'.phase1/ecen21-m8')))
args=argparse.ArgumentParser();args.add_argument('--export-dir',type=Path);args.add_argument('--baseline-generator',type=Path);options=args.parse_args()
OUT=WORK/('hdl-export-verification' if options.export_dir else 'hdl-verification')
GEN=WORK/'host-tests/generate-hdl'
IVERILOG=shutil.which('iverilog') or '/opt/homebrew/bin/iverilog'
VVP=shutil.which('vvp') or '/opt/homebrew/bin/vvp'
if OUT.exists():shutil.rmtree(OUT)
OUT.mkdir(parents=True)
(WORK/'logs').mkdir(exist_ok=True)
REPORT=WORK/('hdl-export-verification.json' if options.export_dir else 'hdl-verification.json')
REPORT.unlink(missing_ok=True)
version_run=subprocess.run([IVERILOG,'-V'],capture_output=True,text=True,timeout=30)
version_run.check_returncode()
version_match=re.search(r'Icarus Verilog version (\S+)',version_run.stdout)
if not version_match:raise RuntimeError('Cannot identify HDL simulator version')
(WORK/'logs/iverilog-version.log').write_text(version_run.stdout+version_run.stderr)
results=[]

def config(kind,**kw):
    c=dict(kind=kind,width=4 if kind==7 else 5,inputs=4,modulus=12,seed=0,slot=0,falling=0,reset=1,high=1,value=0,right=0)
    c.update(kw);return c

def generated(c):
    args=[c[k] for k in ('kind','width','inputs','modulus','seed','slot','falling','reset','high','value','right')]
    if options.baseline_generator and c["kind"]==9:args[0]=0
    p=subprocess.run([str(options.baseline_generator or GEN),*map(str,args)],capture_output=True,check=True)
    assert p.stdout and len(p.stdout)%2==0 and b'\x00' not in p.stdout
    return p.stdout.decode().replace("ecen21_add_b","ecen21_add_d") if options.baseline_generator and c["kind"]==9 else p.stdout.decode()

export_sources={};export_uses={}
if options.export_dir:
    exported=json.loads((ROOT/'apps/ecen21/screenshots/m8-exports.json').read_text())
    if {p.name for p in options.export_dir.glob('*.v')} != set(exported):raise RuntimeError('Wrong exported file set')
    for name,c in exported.items():
        data=(options.export_dir/name).read_text()
        if data.encode()!=generated(c).encode():raise RuntimeError('Target-exported bytes differ from host generator: '+name)
        export_sources[name]=(c,data);export_uses[name]=0

def source(c,exact=False):
    ignored={'slot'}
    candidates=[]
    for name,(settings,data) in export_sources.items():
        if all(c[k]==settings[k] for k in c if k not in ignored):
            score=sum(c[k]==settings[k] for k in c)
            candidates.append((score,name,data))
    if candidates:
        _,name,data=max(candidates);export_uses[name]+=1;return data
    return generated(c)

def check(expr,label='value'):
    return f'if (!({expr})) begin $display("FAIL: {label}"); $finish; end checks=checks+1;\n'

def run(label,dut,declarations,body,extra='',error=False):
    folder=OUT/label;folder.mkdir()
    (folder/'dut.v').write_text(dut)
    (folder/'tb.v').write_text('module tb; integer checks=0;\n'+declarations+'\ninitial begin\n'+body+f'\n$display("PASS: {label}; %0d checks",checks); $finish;\nend\n'+extra+'\nendmodule\n')
    p=subprocess.run([IVERILOG,'-g2001','-Wall','-s','tb','-o',str(folder/'sim.vvp'),str(folder/'dut.v'),str(folder/'tb.v')],capture_output=True,text=True,timeout=30)
    (folder/'compile.log').write_text(p.stdout+p.stderr)
    if p.returncode:raise RuntimeError(label+' compile: '+p.stderr)
    p=subprocess.run([VVP,str(folder/'sim.vvp')],capture_output=True,text=True,timeout=30)
    (folder/'run.log').write_text(p.stdout+p.stderr)
    if p.returncode or 'FAIL:' in p.stdout:raise RuntimeError(label+' simulation: '+p.stdout+p.stderr)
    if error:
        if 'ECEN21 parameter error' not in p.stdout or 'PASS:' in p.stdout:raise RuntimeError(label+' invalid override not rejected')
        checks=1
    else:
        match=re.search(r'^PASS: '+re.escape(label)+r'; (\d+) checks$',p.stdout,re.M)
        if not match or int(match[1])==0:raise RuntimeError(label+' missing passing bench report: '+p.stdout)
        checks=int(match[1])
    results.append(dict(case=label,checks=checks,source_sha256=hashlib.sha256(dut.encode()).hexdigest()))

# Full adders: independent Python wide-integer sums, including5bit review and32bit carry.
rng=random.Random(821)
for w in (1,2,3,5,8,16,31,32):
    mask=(1<<w)-1
    pairs=[(a,b) for a in range(1<<w) for b in range(1<<w)] if w<=5 else [(a,b) for a in (0,1,mask//2,mask//2+1,mask-1,mask) for b in (0,1,mask//2,mask//2+1,mask-1,mask)]+[(rng.randrange(mask+1),rng.randrange(mask+1)) for _ in range(128)]
    declarations=f'reg [{w-1}:0] a,b; reg cin; wire [{w-1}:0] sb,ss,sd;wire cb,cs,cd;\necen21_add_b b_dut(a,b,cin,sb,cb);\necen21_add_s s_dut(a,b,cin,ss,cs);\necen21_add_d d_dut(a,b,cin,sd,cd);'
    body=''
    for a,b in pairs:
        for cin in (0,1):
            total=a+b+cin
            body+=f'a={w}\'d{a};b={w}\'d{b};cin={cin};#1;\n'+check(f"{{cb,sb}} === {w+1}'d{total} && {{cs,ss}} === {w+1}'d{total} && {{cd,sd}} === {w+1}'d{total}",'adder')
    run(f'adder-{w}',source(config(0,width=w))+source(config(1,width=w))+source(config(9,width=w)),declarations,body)

# Ordinary encoder: every binary pattern, including zero/multiple invalid inputs.
for n in (4,8):
    bits=2 if n==4 else 3
    decl=f'reg [{n-1}:0] in;wire [{bits-1}:0] code;wire valid;ecen21_encoder dut(in,code,valid);'
    body=''
    for pattern in range(1<<n):
        code=0
        for i in range(n):
            if pattern&(1<<i):code|=i
        valid=int(pattern.bit_count()==1)
        body+=f'in={n}\'d{pattern};#1;'+check(f"code === {bits}'d{code} && valid === 1'b{valid}",'encoder')
    run(f'encoder-{n}',source(config(2,inputs=n)),decl,body)

# All FF binary event/hold/reset combinations. Uninitialized no-reset T stays X.
for kind in (3,4,5):
 for falling in (0,1):
  for reset in (0,1,2):
   for high in (0,1):
    for value in (0,1):
     c=config(kind,falling=falling,reset=reset,high=high,value=value)
     pin={3:'d',4:'t',5:'j,k'}[kind];module={3:'ecen21_dff',4:'ecen21_tff',5:'ecen21_jkff'}[kind]
     decl=f'reg clk,rst,{pin};wire q;{module} dut(clk,rst,{pin},q);'
     inactive=falling;active=1-falling;deassert=1-high;q='x';body=f'clk={inactive};rst={deassert};'+('j=0;k=0;' if kind==5 else f'{pin}=0;')+'#1;'+check("q === 1'bx",'initial X')
     def observed(expected):return check(f"q === 1'b{expected}",'FF')
     if reset:
        body+=f'rst={high};#1;'+observed(value if reset==2 else q)
        body+=f'clk={active};#1;'+observed(value)+f'clk={inactive};rst={deassert};#1;';q=value
     def advance(old,a,b=0):
        if kind==3:return a
        if kind==4:return 'x' if old=='x' else old^a
        return old if (a,b)==(0,0) else 0 if (a,b)==(0,1) else 1 if (a,b)==(1,0) else 'x' if old=='x' else 1-old
     for iteration in range(4):
      for a in (0,1):
       for b in ((0,1) if kind==5 else (0,)):
        inputs=f'j={a};k={b};' if kind==5 else f'{pin}={a};'
        body+=inputs+'#1;'+observed(q)
        q=advance(q,a,b)
        body+=f'clk={active};#1;'+observed(q)+f'clk={inactive};#1;'+observed(q)
     if kind==5:
      for old in (0,1):
       for a in (0,1):
        for b in (0,1):
         body+=f'j={old};k={1-old};clk={active};#1;'+observed(old)+f'clk={inactive};#1;'
         q=advance(old,a,b)
         body+=f'j={a};k={b};#1;'+observed(old)+f'clk={active};#1;'+observed(q)+f'clk={inactive};#1;'
     # Between-edge reset must act asynchronously only; active edge dominates data.
     body+=f'rst={high};#1;'
     if reset==2:q=value
     body+=observed(q)
     q=value if reset else advance(q,1,1 if kind==5 else 0)
     body+=('j=1;k=1;' if kind==5 else f'{pin}=1;')+f'clk={active};#1;'+observed(q)+f'clk={inactive};rst={deassert};#1;'+observed(q)
     run(f'ff-{kind}-{falling}-{reset}-{high}-{value}',source(c),decl,body)

# Shift: WIDTH1 safe, simultaneous requests, reset timing, serial insertion.
for w in (1,2,5,8,32):
 for falling in (0,1):
  for reset in (0,1,2):
   for high in (0,1):
    mask=(1<<w)-1;c=config(6,width=w,falling=falling,reset=reset,high=high)
    decl=f'reg clk,rst,load,shift_right,shift_left,serial_left,serial_right;reg [{w-1}:0] parallel;wire [{w-1}:0] q;ecen21_shift dut(clk,rst,load,shift_right,shift_left,serial_left,serial_right,parallel,q);'
    inactive=falling;active=1-falling;body=f'clk={inactive};rst={1-high};load=1;shift_right=0;shift_left=0;serial_left=0;serial_right=0;parallel={w}\'d{mask};#1;clk={active};#1;'+check(f"q === {w}'d{mask}",'load')+f'clk={inactive};#1;';q=mask
    for request in range(8):
     for sl in (0,1):
      for sr in (0,1):
       load=request//4;right=(request//2)%2;left=request%2;parallel=rng.randrange(mask+1)
       body+=f'load={load};shift_right={right};shift_left={left};serial_left={sl};serial_right={sr};parallel={w}\'d{parallel};#1;'+check(f"q === {w}'d{q}",'shift hold before edge')
       q=parallel if load else ((q>>1)|(sl<<(w-1))) if right else ((q<<1)|sr)&mask if left else q
       body+=f'clk={active};#1;'+check(f"q === {w}'d{q}",'shift priority')+f'clk={inactive};#1;'
    body+=f'rst={high};#1;'+check(f"q === {w}'d{0 if reset==2 else q}",'shift reset between edge')
    q=0 if reset else mask
    body+=f'load=1;parallel={w}\'d{mask};shift_right=1;shift_left=1;'+f'clk={active};#1;'+check(f"q === {w}'d{q}",'shift reset edge')
    run(f'shift-{w}-{falling}-{reset}-{high}',source(c),decl,body)

# Modulo: source-default width and parameter overrides, sync reset, enable, recovery.
for n in (1,2,3,12,31,65535):
 for falling in (0,1):
  for high in (0,1):
    width=max(1,(n-1).bit_length());c=config(7,width=width,modulus=n,falling=falling,high=high)
    decl=f'reg clk,rst,enable;wire [{width-1}:0] q;integer i,expected;ecen21_modulo dut(clk,rst,enable,q);'
    inactive=falling;active=1-falling;body=f'clk={inactive};rst={high};enable=1;#1;clk={active};#1;'+check(f"q === {width}'d0",'counter reset')+f'clk={inactive};rst={1-high};#1;expected=0;\n'
    body+=f'for(i=0;i<{n+3};i=i+1) begin\nclk={active};#1;expected=(expected+1)%{n};\n'+check('q === expected','counter modulo')+f'clk={inactive};#1;end\n'
    body+=f'enable=0;rst={high};#1;'+check('q === expected','sync reset waits')+f'clk={active};#1;'+check('q === 0','reset overrides enable')+f'clk={inactive};rst={1-high};#1;clk={active};#1;'+check('q === 0','enable hold')
    body+=f'clk={inactive};enable=1;dut.q={width}\'d{(1<<width)-1};#1;clk={active};#1;'+check('q === 0','invalid counter recovery')
    run(f'mod-{n}-{falling}-{high}',source(c),decl,body)
# Extra selected widths retain high bits, recovery and reset semantics.
for width in (8,32):
    c=config(7,width=width,modulus=12)
    decl=f'reg clk,rst,enable;wire [{width-1}:0] q;ecen21_modulo dut(clk,rst,enable,q);'
    body='clk=0;rst=1;enable=1;#1;clk=1;#1;'+check('q === 0','wide reset')+'clk=0;rst=0;#1;clk=1;#1;'+check('q === 1','wide increment')
    body+=f'clk=0;dut.q={width}\'d{(1<<width)-1};#1;clk=1;#1;'+check('q === 0','wide recovery')
    run(f'mod-wide-{width}',source(c),decl,body)

# Ring: one-hot seed, both directions/edges/reset polarities, enable and period.
for w in (1,2,5,8,32):
 for falling in (0,1):
  for reset in (1,2):
   for high in (0,1):
    for right in (0,1):
     for seed in sorted({0,w-1,2 if w==8 else 0}):
      c=config(8,width=w,falling=falling,reset=reset,high=high,right=right,seed=seed);mask=(1<<w)-1;q=1<<seed
      decl=f'reg clk,rst,enable;wire [{w-1}:0] q;ecen21_ring dut(clk,rst,enable,q);'
      inactive=falling;active=1-falling;body=f'clk={inactive};rst={1-high};enable=1;#1;rst={high};#1;'+check(f"q === {w}'d{q}" if reset==2 else "q === {"+f"{w}{{1'bx}}"+"}",'ring initial reset timing')
      body+=f'clk={active};#1;'+check(f"q === {w}'d{q}",'ring seed')+f'clk={inactive};rst={1-high};#1;'
      for i in range(w*2):
        q=((q>>1)|(q<<(w-1)))&mask if right else ((q<<1)|(q>>(w-1)))&mask
        body+=f'clk={active};#1;'+check(f"q === {w}'d{q}",'ring rotation')+check('(q != 0) && ((q & (q-1)) == 0)','one hot')+f'clk={inactive};#1;'
      body+=f'enable=0;clk={active};#1;'+check(f"q === {w}'d{q}",'ring hold')
      run(f'ring-{w}-{falling}-{reset}-{high}-{right}-{seed}',source(c),decl,body)

# Validate emitted modeling forms separately from behavioral simulation.
if not options.baseline_generator:
    for k in range(10):
        code=generated(config(k))
        assert not any(token in code for token in ('parameter','initial','$display','$finish','translate_','//'))
        if k==0:assert 'always @(*)' in code and 'output reg' in code and 'assign ' not in code
        if k==9:assert 'assign {cout,sum}' in code and 'always' not in code and 'reg ' not in code
        if k==1:assert all(token in code for token in ('xor (','and (','or (','ecen21_fa1 fa(')) and 'always' not in code

summary=dict(simulator='Icarus Verilog'+version_match[1],language='Verilog-2001 (-g2001)',cases=len(results),checks=sum(r['checks'] for r in results),results=results,physical_fpga_tested=False)
if options.export_dir and not all(export_uses.values()):raise RuntimeError('Unsimulated exported source: '+str(export_uses))
summary['exported_files']=len(export_sources)
summary['export_source_uses']=export_uses
REPORT.write_text(json.dumps(summary,indent=2)+'\n')
print(json.dumps({k:v for k,v in summary.items() if k!='results'},indent=2))
