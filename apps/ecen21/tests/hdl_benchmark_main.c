/* Test-only representative largest-parameter generation/paging. */
#include "hdl.h"
#include <gint/bfile.h>
#include <gint/gint.h>
#include <stdio.h>
#include <string.h>
static hg_document doc;
static char report[192];
static int save(void)
{
    uint16_t const path[]=u"\\\\fls0\\M8BENCH.txt";
    int size=(int)strlen(report);BFile_Remove(path);
    if(BFile_Create(path,BFile_File,&size)<0)return -1;
    int fd=BFile_Open(path,BFile_WriteOnly);if(fd<0)return -1;
    int rc=BFile_Write(fd,report,size);int closed=BFile_Close(fd);return rc!=size||closed<0?-1:0;
}
int main(void)
{
    bool ok=true;unsigned bytes=0,lines=0,count=0;
    for(unsigned kind=0;kind<HG_KIND_COUNT;kind++)for(unsigned fall=0;fall<2;fall++)for(unsigned high=0;high<2;high++) {
        hg_config c;hg_init(&c,(hg_kind)kind);c.width=32;c.inputs=8;c.modulus=65535;c.seed=31;c.slot=99;c.falling=fall!=0;c.reset_high=high!=0;c.reset_value=true;c.right=fall!=0;
        ok=ok&&hg_generate(&c,&doc)&&doc.valid&&!(doc.length&1u)&&strlen(doc.code)==doc.length;
        if(doc.length>bytes)bytes=doc.length;
        unsigned n=hg_pages_lines(&doc);if(n>lines)lines=n;
        char text[47];for(unsigned line=0;line<n;line++)ok=ok&&hg_line(&doc,line,text,sizeof text)&&strlen(text)<=46;
        count++;
    }
    snprintf(report,sizeof report,"%s: M8 maximum families; %u configs; peak bytes %u; peak visual lines %u\n",ok?"PASS":"FAIL",count,bytes,lines);
    if(strlen(report)&1u)strcat(report,"\n");
    int saved=gint_world_switch(GINT_CALL(save));return !ok||saved?1:0;
}
