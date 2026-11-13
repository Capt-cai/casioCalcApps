/* Test-only whole-program instruction bound for the largest M5 UI example.
 * Includes initialization, solving, wiring, validation and report I/O, so the
 * emulator's instruction total is an upper bound on this one solve path,
 * not a pure solver latency measurement or a worst-case arbitrary input claim. */
#include "sequential.h"
#include "seq_connections.h"
#include <gint/bfile.h>
#include <gint/gint.h>
#include <stdio.h>
#include <string.h>

static sq_model model;
static cn_netlist net;
static char report[160];
static int save(void)
{
    uint16_t const path[]=u"\\\\fls0\\M5BENCH.txt";
    int size=(int)strlen(report);BFile_Remove(path);
    if(BFile_Create(path,BFile_File,&size)<0)return -1;
    int fd=BFile_Open(path,BFile_WriteOnly);if(fd<0)return -1;
    int rc=BFile_Write(fd,report,size);BFile_Close(fd);return rc<0?-1:0;
}
int main(void)
{
    static const unsigned expected[16]={3,1,0,4,0,5,5,2,0,7,7,0,0,3,0,6};
    sq_init(&model);
    sq_set_controlled(&model,true);sq_set_type(&model,2,FF_D);sq_set_type(&model,0,FF_JK);
    sq_set_text(&model,0,"03570");sq_set_text(&model,1,"014763250");
    bool ok=sq_solve(&model) && model.count==16 && model.checked_rows==32;
    if(ok)for(unsigned r=0;r<16;r++)for(unsigned f=0;f<2;f++)if(model.rows[r].next[f]!=expected[r])ok=false;
    if(ok)ok=cn_build(&model,EB_SOP,&net) && net.valid;
    snprintf(report,sizeof report,"%s: M5 maximum-domain; %u rows; %u form-row checks; %u nodes\n",
        ok?"PASS":"FAIL",model.count,model.checked_rows,net.count);
    int saved=gint_world_switch(GINT_CALL(save));
    return ok && !saved?1:0;
}
