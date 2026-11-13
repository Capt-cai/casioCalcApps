/* Test-only maximum-family exercise. Not linked into production. */
#include "combinational.h"
#include <gint/bfile.h>
#include <gint/gint.h>
#include <stdio.h>
#include <string.h>
static cb_model model;
static char report[256];
static int save(void)
{
    uint16_t const path[]=u"\\\\fls0\\M7BENCH.txt";
    int size=(int)strlen(report);BFile_Remove(path);
    if(BFile_Create(path,BFile_File,&size)<0)return -1;
    int fd=BFile_Open(path,BFile_WriteOnly);if(fd<0)return -1;
    int rc=BFile_Write(fd,report,size);BFile_Close(fd);return rc<0?-1:0;
}
int main(void)
{
    bool ok=true;unsigned peak=0;
    cb_init(&model,CB_FUNCTION);model.variables=4;model.on=0x6996;model.select_count=3;
    ok=ok&&cb_solve(&model);peak=model.node_count;
    for(unsigned row=0;row<16;row++) {
        unsigned ones=0;for(unsigned i=0;i<4;i++)ones+=(row>>i)&1;
        ok=ok&&cb_eval_gate(&model,row,false)==(int)(ones%2)&&cb_eval_mux(&model,row)==(int)(ones%2);
    }
    for(unsigned out=0;out<3;out++) {
        cb_init(&model,CB_ENCODER);model.encoder_inputs=8;model.output=out;
        ok=ok&&cb_solve(&model);if(model.node_count>peak)peak=model.node_count;
        for(unsigned row=0;row<256;row++){bool one=row&&!(row&(row-1));ok=ok&&cb_eval_gate(&model,row,true)==(int)one;}
    }
    for(unsigned low=0;low<2;low++)for(unsigned policy=0;policy<3;policy++)for(unsigned out=0;out<7;out++) {
        cb_init(&model,CB_DISPLAY);model.active_low=low!=0;model.invalid_policy=policy;model.output=out;
        ok=ok&&cb_solve(&model);if(model.node_count>peak)peak=model.node_count;
    }
    cb_stage stages[8];unsigned sum,cout;
    ok=ok&&cb_ripple(8,255,255,1,stages,&sum,&cout)&&sum==255&&cout==1;
    snprintf(report,sizeof report,"%s: M7 maximum families; 4vars; 8:1MUX; 8input encoder; 7segments; 8bit adder; peak nodes %u\n",ok?"PASS":"FAIL",peak);
    int saved=gint_world_switch(GINT_CALL(save));return !ok||saved?1:0;
}
