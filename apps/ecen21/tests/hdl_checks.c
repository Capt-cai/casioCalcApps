#include "hdl.h"
#include "hdl_export.h"
#include "hdl_checks.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { ++*checks;if(!(x))return __LINE__; } while(0)
static hg_document doc;
typedef struct { int failure;bool present,removed;unsigned length,opens,closes,writes;char data[HG_CAPACITY]; } storage;
static storage disk;
static int exists(void *ctx,const char *name) { storage *s=ctx;return !name||s->failure==1?-44:s->present?1:0; }
static int create(void *ctx,const char *name,unsigned length) { storage *s=ctx;(void)name;if(s->failure==2)return -12;s->present=true;s->length=length;return 0; }
static int open_file(void *ctx,const char *name) { storage *s=ctx;(void)name;s->opens++;return s->failure==3?-3:7; }
static int write_file(void *ctx,int fd,const char *data,unsigned length) { storage *s=ctx;s->writes++;if(fd!=7||s->failure==4||s->failure==7)return -4;memcpy(s->data,data,length);return s->failure==6?(int)length-2:(int)length; }
static int close_file(void *ctx,int fd) { storage *s=ctx;s->closes++;return fd!=7||s->failure==5?-5:0; }
static int remove_file(void *ctx,const char *name) { storage *s=ctx;(void)name;if(s->failure==7)return -6;s->present=false;s->removed=true;return 0; }
static int line_checks(unsigned *checks)
{
    unsigned raw=0;for(unsigned i=0;i<doc.length;i++){if(doc.code[i]=='\n')raw=0;else {raw++;CHECK(raw<=HG_LINE_WIDTH);}}
    unsigned visual=0,column=0;char actual[47],expected[47];
    for(unsigned i=0;i<doc.length;i++) {
        char c=doc.code[i];
        if(c=='\n'||column==46) {
            expected[column]=0;CHECK(hg_line(&doc,visual++,actual,sizeof actual));CHECK(!strcmp(actual,expected));column=0;
            if(c=='\n')continue;
        }
        CHECK(column<46);expected[column++]=c;
    }
    if(column){expected[column]=0;CHECK(hg_line(&doc,visual++,actual,sizeof actual));CHECK(!strcmp(actual,expected));}
    CHECK(visual==hg_pages_lines(&doc));CHECK(!hg_line(&doc,visual,actual,sizeof actual));
    CHECK(!hg_line(&doc,0,actual,46));return 0;
}
int hg_run_checks(unsigned *checks)
{
    *checks=0;hg_config c;char filename[16];
    for(unsigned kind=0;kind<HG_KIND_COUNT;kind++)for(unsigned width=1;width<=32;width++) {
        hg_init(&c,(hg_kind)kind);c.width=width;c.modulus=width<16?(1u<<width):65535;c.seed=width-1;
        for(unsigned fall=0;fall<2;fall++)for(unsigned high=0;high<2;high++) {
            c.falling=fall!=0;c.reset_high=high!=0;c.reset_value=high!=0;c.right=fall!=0;
            CHECK(hg_generate(&c,&doc));CHECK(doc.valid&&doc.length>0&&doc.length<HG_CAPACITY&&!(doc.length&1u));
            CHECK(strlen(doc.code)==doc.length);CHECK(strstr(doc.code,"endmodule\n")!=NULL);
            CHECK(hg_filename(&c,filename,sizeof filename));CHECK(strlen(filename)<=12);
            if(kind>=HG_D&&kind<=HG_RING)CHECK(strstr(doc.code,fall?"negedge clk":"posedge clk")!=NULL);
            CHECK(!strstr(doc.code,"parameter")&&!strstr(doc.code,"initial")&&!strstr(doc.code,"translate_off")&&!strstr(doc.code,"//"));
            if(kind==HG_BEHAVIORAL)CHECK(strstr(doc.code,"always @(*)")&&!strstr(doc.code,"assign"));
            if(kind==HG_DATAFLOW)CHECK(strstr(doc.code,"assign {cout,sum}")&&!strstr(doc.code,"always"));
            if(kind==HG_STRUCTURAL)CHECK(strstr(doc.code,"xor (")&&strstr(doc.code,"and (")&&strstr(doc.code,"or (")&&strstr(doc.code,"ecen21_fa1 fa(")&&!strstr(doc.code,"always"));
            int rc=line_checks(checks);if(rc)return rc;
        }
    }
    for(unsigned kind=HG_D;kind<=HG_SHIFT;kind++)for(unsigned reset=0;reset<3;reset++) {
        hg_init(&c,(hg_kind)kind);c.reset=(hg_reset)reset;CHECK(hg_generate(&c,&doc));
        CHECK((strstr(doc.code,"or posedge rst")!=NULL)==(reset==HG_ASYNC_RESET));
        CHECK((strstr(doc.code,"if (rst)")!=NULL)==(reset!=HG_NO_RESET));
    }
    hg_init(&c,HG_BEHAVIORAL);CHECK(hg_generate(&c,&doc));c.width=0;CHECK(!hg_generate(&c,&doc)&&!doc.valid&&!doc.length&&!doc.code[0]);
    c.width=33;CHECK(!hg_generate(&c,&doc));c.width=5;c.slot=100;CHECK(!hg_generate(&c,&doc));
    c.slot=99;CHECK(hg_filename(&c,filename,sizeof filename)&&!strcmp(filename,"ECAB99.v"));
    CHECK(!hg_filename(&c,filename,3));CHECK(!hg_generate(NULL,&doc));CHECK(!hg_generate(&c,NULL));
    hg_init(&c,(hg_kind)99);CHECK(!hg_generate(&c,&doc));
    hg_init(&c,HG_ENCODER);c.inputs=5;CHECK(!hg_generate(&c,&doc));c.inputs=8;CHECK(hg_generate(&c,&doc));
    hg_init(&c,HG_MODULO);c.width=3;CHECK(!hg_generate(&c,&doc));c.width=4;c.modulus=0;CHECK(!hg_generate(&c,&doc));
    c.modulus=65536;CHECK(!hg_generate(&c,&doc));c.modulus=12;c.reset=HG_ASYNC_RESET;CHECK(!hg_generate(&c,&doc));
    hg_init(&c,HG_RING);c.seed=c.width;CHECK(!hg_generate(&c,&doc));c.seed=0;c.reset=HG_NO_RESET;CHECK(!hg_generate(&c,&doc));
    hg_init(&c,HG_D);c.reset=(hg_reset)99;CHECK(!hg_generate(&c,&doc));
    CHECK(hg_min_width(1)==1&&hg_min_width(2)==1&&hg_min_width(3)==2&&hg_min_width(12)==4&&hg_min_width(65535)==16);
    hg_init(&c,HG_BEHAVIORAL);CHECK(hg_generate(&c,&doc));
    hx_io io={&disk,exists,create,open_file,write_file,close_file,remove_file};
    memset(&disk,0,sizeof disk);hx_result result=hx_export(&c,&doc,&io);
    CHECK(result.status==HX_OK&&disk.present&&disk.closes==1&&disk.writes==1);
    CHECK(disk.length==doc.length&&!memcmp(disk.data,doc.code,doc.length));
    result=hx_export(&c,&doc,&io);CHECK(result.status==HX_EXISTS&&disk.closes==1&&disk.writes==1&&!disk.removed);
    for(int failure=1;failure<=7;failure++) {
        memset(&disk,0,sizeof disk);disk.failure=failure;result=hx_export(&c,&doc,&io);
        hx_status expected=failure==1?HX_LOOKUP:failure==2?HX_CREATE:failure==3?HX_OPEN:failure==5?HX_CLOSE:HX_WRITE;
        CHECK(result.status==expected);CHECK(result.cleanup_failed==(failure==7));
        if(failure>=3)CHECK(disk.removed==(failure!=7));
        if(failure>=4)CHECK(disk.closes==1);
    }
    c.width=6;CHECK(hx_export(&c,&doc,&io).status==HX_BAD);c.width=5;
    c.slot=99;CHECK(hg_matches(&c,&doc));
    doc.length++;CHECK(hx_export(&c,&doc,&io).status==HX_BAD);doc.length--;
    char first=doc.code[0];doc.code[0]=0;CHECK(hx_export(&c,&doc,&io).status==HX_BAD);doc.code[0]=first;
    doc.valid=false;memset(&disk,0,sizeof disk);CHECK(hx_export(&c,&doc,&io).status==HX_BAD&&!disk.present);
    CHECK(hx_export(&c,&doc,NULL).status==HX_BAD);
    return 0;
}
