#!/usr/bin/env python3
"""Audit the existing Logic core on the host without changing calculator code.

Extracts the original static functions verbatim, compiles with Apple Clang
and sanitizers, and checks truth-table equivalence and cover optimality.
This does not substitute for a SuperH build or calculator runtime test.
"""
from pathlib import Path
import hashlib
import subprocess

root = Path(__file__).resolve().parents[1]
source = root / "apps/logic/src/main.c"
text = source.read_text()
core = text[text.index("#define MAX_VARS"):text.index("//  Views")]
# The section divider immediately before Views is an unfinished comment.
core = core[:core.rfind("//---")]
out = root / ".phase1/core-check"
out.mkdir(parents=True, exist_ok=True)
harness = r'''
#include <stdlib.h>
static unsigned char optimum[65536];
int main(void) {
    struct { const char *s; unsigned mask; } examples[] = {
        {"AB + C", 0xea}, {"A^B", 0x6}, {"A+A'", 0xf},
        {"A.!B", 0x4}, {"!(A+B)", 0x1}, {"1", 0xf}, {"0", 0},
        {"(A+B)(A+C)", 0xf8}, {"D", 0xaaaa},
    };
    for(unsigned i=0; i<sizeof examples/sizeof *examples; i++) {
        model_t m = {.from_expr=true};
        strcpy(m.expr, examples[i].s);
        recompute(&m);
        if(m.error || m.minterms != examples[i].mask) {
            fprintf(stderr,"expression failed: %s -> %04x\n",m.expr,m.minterms);
            return 1;
        }
    }
    const char *invalid[] = {"", "A+", "(A", "E", "A)"};
    for(unsigned i=0; i<sizeof invalid/sizeof *invalid; i++)
        if(eval(invalid[i],0,NULL) != -1) return 2;

    /* Independent exact minimum-term oracle: every possible 4-variable
       cube, then shortest paths over unions of their truth masks. */
    unsigned cubes[81]; int nc=0;
    for(unsigned fixed=0; fixed<16; fixed++)
        for(unsigned bits=0; bits<16; bits++) {
            if(bits & ~fixed) continue;
            unsigned mask=0;
            for(unsigned r=0;r<16;r++)
                if((r & fixed)==bits) mask |= 1u<<r;
            cubes[nc++]=mask;
        }
    memset(optimum,255,sizeof optimum); optimum[0]=0;
    for(unsigned mask=0;mask<65536;mask++) {
        if(optimum[mask]==255) return 3;
        for(int c=0;c<nc;c++) {
            unsigned next=mask|cubes[c];
            if(optimum[next]>optimum[mask]+1)
                optimum[next]=optimum[mask]+1;
        }
    }
    unsigned checked=0, nonminimal=0, truncated=0, badtext=0;
    for(int vc=2;vc<=4;vc++) {
        unsigned limit=1u<<(1<<vc);
        for(unsigned mask=0;mask<limit;mask++) {
            model_t m={.var_count=vc,.minterms=mask};
            cover_t cov; minimal_cover(&m,&cov);
            unsigned got=0;
            for(int i=0;i<cov.count;i++)
                for(int r=0;r<(1<<vc);r++)
                    if(covers(cov.items[i],r)) got |= 1u<<r;
            if(got != mask) {
                fprintf(stderr,"cover mismatch: vc=%d mask=%04x got=%04x\n",vc,mask,got);
                return 4;
            }
            if(vc==4 && cov.count>optimum[mask]) {
                if(nonminimal++==0)
                    printf("first nonminimal cover: mask=0x%04x greedy=%d optimum=%d\n",
                           mask,cov.count,optimum[mask]);
            }
            char sop[80]; sop_text(sop,sizeof sop,&m,&cov);
            char full[512]; sop_text(full,sizeof full,&m,&cov);
            if(strcmp(sop,full)) {
                if(truncated++==0)
                    printf("first truncated SOP: vc=%d mask=0x%04x length=%zu\n",vc,mask,strlen(full));
            }
            bool bad=false;
            for(int r=0;r<(1<<vc);r++)
                if(eval(sop,r<<(4-vc),NULL) != (int)((mask>>r)&1)) bad=true;
            if(bad) badtext++;
            checked++;
        }
    }
    printf("expression examples: 9 valid, 5 invalid passed\n");
    printf("cover equivalence: %u tables passed (all 2-4 variable functions)\n",checked);
    printf("4-variable nonminimal covers: %u / 65536\n",nonminimal);
    printf("truncated 80-byte SOP buffers: %u; SOP round-trip failures: %u\n",truncated,badtext);
    /* Findings are reported, not hidden as a failed infrastructure run. */
    return 0;
}
'''
generated = out / "logic-core-check.c"
generated.write_text("#include <stdbool.h>\n#include <stdint.h>\n#include <stdio.h>\n#include <string.h>\n" + core + harness)
print("Logic source SHA-256:", hashlib.sha256(source.read_bytes()).hexdigest(), flush=True)
subprocess.run(["clang", "-std=c11", "-O1", "-g", "-fsanitize=address,undefined",
                "-fno-omit-frame-pointer", str(generated), "-o", str(out / "logic-core-check")], check=True)
subprocess.run([str(out / "logic-core-check")], check=True)
