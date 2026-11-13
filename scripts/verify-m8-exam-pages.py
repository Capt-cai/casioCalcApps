#!/usr/bin/env python3
"""Visit every visual source page for the ten M8 exam modeling variants."""
import hashlib,json,os,subprocess
from pathlib import Path
from PIL import Image,ImageChops
ROOT=Path(__file__).resolve().parents[1]
WORK=Path(os.environ.get('ECEN21_M8_WORK',str(ROOT/'.phase1/ecen21-m8-exam')))
OUT=WORK/'complete-code-pages';OUT.mkdir(parents=True,exist_ok=True)
GEN=WORK/'host-tests/generate-hdl';EMU=ROOT/'.phase1/emulator-build/emucasiocalc'
script=['wait 500','7'];records=[]
for kind in range(10):
    script+=['F5',str(kind+1 if kind<9 else 1)]
    config=[kind,4 if kind==7 else 5,4,12,0,0,0,1,1,0,0]
    if kind in (0,1,6,7,8,9):
        script+=['EXE','3','2','EXE'];config[1]=32
    if kind==2:script+=['EXE'];config[2]=8
    if kind==7:script+=['DOWN','EXE','6','5','5','3','5','EXE'];config[3]=65535
    if kind==9:script+=['DOWN','DOWN','EXE']
    source=subprocess.check_output([str(GEN),*map(str,config)])
    lines=sum(max(1,(len(line)+45)//46) for line in source.decode().splitlines())
    pages=(lines+6)//7
    script+=['F6']
    paths=[]
    for page in range(pages):
        name=f'kind{kind}-page{page+1}.png';paths.append(name)
        script+=['shot '+str(OUT/name),'RIGHT']
    script+=['shot '+str(OUT/f'kind{kind}-roundtrip.png')]
    records.append(dict(kind=kind,pages=pages,visual_lines=lines,source_sha256=hashlib.sha256(source).hexdigest(),screenshots=paths))
script+=['EXIT','EXIT','wait 300']
path=OUT/'run.script';path.write_text('\n'.join(script)+'\n')
p=subprocess.run([str(EMU),'--headless','--clock','2026-01-01 12:00','--script',str(path),str(WORK/'artifacts/ECEN21.g3a')],cwd=OUT,capture_output=True,text=True,timeout=120)
(OUT/'emulator.log').write_text(p.stdout+p.stderr);p.check_returncode()
assert 'add-in exited after' in p.stdout and 'stopped with an error' not in p.stdout
for record in records:
    for name in record['screenshots']:
        im=Image.open(OUT/name).convert('RGB');assert im.size==(792,448)
    first=Image.open(OUT/record['screenshots'][0]).convert('RGB')
    last=Image.open(OUT/f"kind{record['kind']}-roundtrip.png").convert('RGB')
    assert ImageChops.difference(first,last).getbbox() is None,record
report=dict(variants=len(records),source_pages=sum(r['pages'] for r in records),page_wrap_checks=len(records),records=records)
(OUT/'report.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({k:v for k,v in report.items() if k!='records'}))
