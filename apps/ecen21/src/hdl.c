/* Bounded Verilog-2001 source generator; existing solver cores are unchanged.
 * Constants/characteristic semantics match the verified adder/encoder/FF tools. */
#include "hdl.h"
#include "flipflop.h"
#include "combinational.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static const char *const names[]={"Behavioral adder","Structural adder","Ordinary encoder","D flip-flop","T flip-flop","JK flip-flop","Universal shift register","Modulo-N counter","One-hot ring counter","Dataflow adder"};
static const char *const modules[]={"ecen21_add_b","ecen21_add_s","ecen21_encoder","ecen21_dff","ecen21_tff","ecen21_jkff","ecen21_shift","ecen21_modulo","ecen21_ring","ecen21_add_d"};
bool hg_is_adder(hg_kind k) { return k==HG_BEHAVIORAL||k==HG_STRUCTURAL||k==HG_DATAFLOW; }
const char *hg_name(hg_kind k) { return (unsigned)k<HG_KIND_COUNT?names[k]:"Invalid family"; }
const char *hg_module(hg_kind k) { return (unsigned)k<HG_KIND_COUNT?modules[k]:"invalid"; }
void hg_init(hg_config *c,hg_kind k)
{
    if(!c)return;
    *c=(hg_config){.kind=k,.width=k==HG_MODULO?4:5,.inputs=4,.modulus=12,.reset_high=true,.reset=HG_SYNC_RESET};
}
unsigned hg_min_width(unsigned n)
{ unsigned width=1;while(width<32&&((unsigned long long)1<<width)<n)width++;return width; }
static void emit(hg_document *d,const char *format,...)
{
    if(d->error[0])return;
    va_list args;va_start(args,format);
    int n=vsnprintf(d->code+d->length,sizeof d->code-d->length,format,args);va_end(args);
    if(n<0||(unsigned)n>=sizeof d->code-d->length){snprintf(d->error,sizeof d->error,"Generated source exceeds8191 bytes");return;}
    d->length+=(unsigned)n;
}
static bool reject(hg_document *d,const char *why)
{ d->valid=false;d->length=0;d->code[0]=0;snprintf(d->error,sizeof d->error,"%s",why);return false; }
static void always_begin(const hg_config *c,hg_document *d,const char *reset_expression)
{
    emit(d,"always @(%s clk",c->falling?"negedge":"posedge");
    if(c->reset==HG_ASYNC_RESET)emit(d," or %s rst",c->reset_high?"posedge":"negedge");
    emit(d,")\n");
    if(c->reset!=HG_NO_RESET)emit(d,"  if (%srst) q <= %s;\n  else ",c->reset_high?"":"!",reset_expression);
    else emit(d,"  ");
}
static void always_end(hg_document *d) { emit(d,"endmodule\n"); }
bool hg_generate(const hg_config *c,hg_document *d)
{
    if(!d)return false;
    memset(d,0,sizeof *d);
    if(!c)return reject(d,"Missing generator settings");
    if((unsigned)c->kind>=HG_KIND_COUNT)return reject(d,"Invalid HDL family");
    if(c->width<1||c->width>32||c->slot>99)return reject(d,"Width1..32; export slot0..99");
    if((unsigned)c->reset>HG_ASYNC_RESET)return reject(d,"Invalid reset mode");
    if(c->kind==HG_ENCODER&&c->inputs!=4&&c->inputs!=8)return reject(d,"Encoder inputs must be4 or8");
    if(c->kind==HG_MODULO&&(c->modulus<1||c->modulus>65535||c->width<hg_min_width(c->modulus)||c->reset!=HG_SYNC_RESET))return reject(d,"Modulo1..65535; sufficient width; SYNC reset");
    if(c->kind==HG_RING&&(c->seed>=c->width||c->reset==HG_NO_RESET))return reject(d,"Ring seed<width; reset required");
    if(hg_is_adder(c->kind)) {
        if(c->kind==HG_STRUCTURAL) {
            emit(d,"module ecen21_fa1(input a,b,cin, output sum,cout);\nwire p,g,h;\nxor (p,a,b), (sum,p,cin);\nand (g,a,b), (h,p,cin);\nor (cout,g,h);\nendmodule\n\n");
        }
        emit(d,"module %s(input [%u:0] a,b, input cin,\n  output %s[%u:0] sum, output %scout);\n",hg_module(c->kind),c->width-1,c->kind==HG_BEHAVIORAL?"reg ":"",c->width-1,c->kind==HG_BEHAVIORAL?"reg ":"");
        if(c->kind==HG_BEHAVIORAL)emit(d,"always @(*) {cout,sum} = a+b+cin;\n");
        else if(c->kind==HG_DATAFLOW)emit(d,"assign {cout,sum} = a+b+cin;\n");
        else emit(d,"wire [%u:0] carry;\nassign carry[0]=cin;\nassign cout=carry[%u];\ngenvar i;\ngenerate for(i=0;i<%u;i=i+1) begin:stage\n  ecen21_fa1 fa(a[i],b[i],carry[i],sum[i],carry[i+1]);\nend endgenerate\n",c->width,c->width,c->width);
        emit(d,"endmodule\n");
    }
    else if(c->kind==HG_ENCODER) {
        emit(d,"module ecen21_encoder(input [%u:0] in,\n  output reg [%u:0] code, output reg valid);\ninteger i,ones;\nalways @(*) begin\n  code=0; ones=0;\n  for(i=0;i<%u;i=i+1)\n    if(in[i]) begin code=code|i; ones=ones+1; end\n  valid=(ones==1);\nend\nendmodule\n",c->inputs-1,c->inputs==8?2:1,c->inputs);
    }
    else if(c->kind<=HG_JK) {
        const char *ports=c->kind==HG_D?"d":c->kind==HG_T?"t":"j,k";
        emit(d,"module %s(input clk,rst,%s, output reg q);\n",hg_module(c->kind),ports);
        always_begin(c,d,c->reset_value?"1'b1":"1'b0");
        if(c->kind==HG_D)emit(d,"q <= d;\n");
        if(c->kind==HG_T)emit(d,"q <= q ^ t;\n");
        if(c->kind==HG_JK)emit(d,"case ({j,k})\n    2'b00: q <= q;\n    2'b01: q <= 0;\n    2'b10: q <= 1;\n    2'b11: q <= ~q;\n    default: q <= 1'bx;\n  endcase\n");
        always_end(d);
    }
    else if(c->kind==HG_SHIFT) {
        emit(d,"module ecen21_shift(\n  input clk,rst,load,shift_right,shift_left,\n  input serial_left,serial_right,\n  input [%u:0] parallel, output reg [%u:0] q);\n",c->width-1,c->width-1);
        always_begin(c,d,"0");
        emit(d,"if(load) q <= parallel;\n  else if(shift_right) q <= ");
        if(c->width==1)emit(d,"serial_left;\n");else emit(d,"{serial_left,q[%u:1]};\n",c->width-1);
        emit(d,"  else if(shift_left) q <= ");
        if(c->width==1)emit(d,"serial_right;\n");else emit(d,"{q[%u:0],serial_right};\n",c->width-2);
        always_end(d);
    }
    else if(c->kind==HG_MODULO) {
        emit(d,"module ecen21_modulo(input clk,rst,enable,\n  output reg [%u:0] q);\n",c->width-1);
        always_begin(c,d,"0");
        emit(d,"if(enable)\n    if(q>=%u) q <= 0;\n    else q <= q+1'b1;\n",c->modulus-1);
        always_end(d);
    }
    else {
        char seed[48];snprintf(seed,sizeof seed,"%u'd%llu",c->width,1ULL<<c->seed);
        emit(d,"module ecen21_ring(input clk,rst,enable,\n  output reg [%u:0] q);\n",c->width-1);
        always_begin(c,d,seed);
        emit(d,"if(enable) q <= ");
        if(c->width==1)emit(d,"q;\n");
        else if(c->right)emit(d,"{q[0],q[%u:1]};\n",c->width-1);
        else emit(d,"{q[%u:0],q[%u]};\n",c->width-2,c->width-1);
        always_end(d);
    }
    if(d->error[0])return reject(d,"Generated source exceeds capacity");
    if(d->length&1u)emit(d,"\n");
    if(d->error[0])return reject(d,"Generated source exceeds capacity");
    d->generated=*c;d->valid=true;return true;
}
bool hg_matches(const hg_config *c,const hg_document *d)
{
    if(!c||!d||!d->valid)return false;
    const hg_config *p=&d->generated;
    return c->kind==p->kind&&c->width==p->width&&c->inputs==p->inputs&&
        c->modulus==p->modulus&&c->seed==p->seed&&c->falling==p->falling&&
        c->reset==p->reset&&c->reset_high==p->reset_high&&
        c->reset_value==p->reset_value&&c->right==p->right;
}
bool hg_filename(const hg_config *c,char *out,size_t cap)
{
    static const char *const prefix[]={"ECAB","ECAS","ECEN","ECDF","ECTF","ECJK","ECSR","ECMN","ECRG","ECAD"};
    if(!c||!out||!cap||(unsigned)c->kind>=HG_KIND_COUNT||c->slot>99){if(out&&cap)out[0]=0;return false;}
    int n=snprintf(out,cap,"%s%02u.v",prefix[c->kind],c->slot);return n>=0&&(size_t)n<cap;
}
/* Visual wrapping never modifies source bytes or drops indentation/characters. */
unsigned hg_pages_lines(const hg_document *d)
{
    if(!d||!d->valid||d->length>=HG_CAPACITY)return 0;
    unsigned lines=0,column=0;
    for(unsigned i=0;i<d->length;i++) {
        if(d->code[i]=='\n'){lines++;column=0;}
        else {if(column==HG_LINE_WIDTH){lines++;column=0;}column++;}
    }
    return lines+(column?1:0);
}
bool hg_line(const hg_document *d,unsigned wanted,char *out,size_t cap)
{
    if(!d||!d->valid||!out||cap<HG_LINE_WIDTH+1||wanted>=hg_pages_lines(d))return false;
    unsigned line=0,column=0;
    for(unsigned i=0;i<d->length;i++) {
        char c=d->code[i];
        if(c=='\n'){if(line==wanted){out[column]=0;return true;}line++;column=0;continue;}
        if(column==HG_LINE_WIDTH){if(line==wanted){out[column]=0;return true;}line++;column=0;}
        if(line==wanted)out[column]=c;
        column++;
    }
    out[column]=0;return line==wanted;
}
