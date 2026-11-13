/* Test-only largest M6 event matrix; whole-program instructions include I/O. */
#include "timing_samples.h"
#include <gint/bfile.h>
#include <gint/gint.h>
#include <stdio.h>
#include <string.h>
static tm_model model;
static tm_phase_log phases;
static char report[160];
static int save(void)
{
    uint16_t const path[]=u"\\\\fls0\\M6BENCH.txt";int size=(int)strlen(report);BFile_Remove(path);
    if(BFile_Create(path,BFile_File,&size)<0)return -1;
    int fd=BFile_Open(path,BFile_WriteOnly);if(fd<0)return -1;
    int rc=BFile_Write(fd,report,size);BFile_Close(fd);return rc<0?-1:0;
}
int main(void)
{
    bool ok=tm_sample(&model,0) && tm_run(&model) && model.completed==65;
    if(ok)for(unsigned c=1;c<=64;c++)for(unsigned bit=0;bit<4;bit++){
        unsigned expected=(((c+1)/2)%16>>bit)&1u;if(model.values[c][4+bit]!=expected)ok=false;
    }
    if(ok)ok=tm_explain(&model,64,&phases);
    snprintf(report,sizeof report,"%s: M6 maximum-domain; %u columns; 12 signals; 4 cells\n",ok?"PASS":"FAIL",model.completed);
    int saved=gint_world_switch(GINT_CALL(save));return ok && !saved?1:0;
}
