/* Bounded Verilog-2001 source generator; existing solver cores are unchanged.
 * Constants/characteristic semantics match the verified adder/encoder/FF tools. */
#include "hdl.h"
#include "flipflop.h"
#include "combinational.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static const char *const names[]={"Behavioral full adder","Structural full adder","Ordinary encoder","D flip-flop","T flip-flop","JK flip-flop","Universal shift register","Modulo-N counter","One-hot ring counter"};
static const char *const modules[]={"ecen21_add_b","ecen21_add_s","ecen21_encoder","ecen21_dff","ecen21_tff","ecen21_jkff","ecen21_shift","ecen21_modulo","ecen21_ring"};
const char *hg_name(hg_kind k) { return (unsigned)k<=HG_RING?names[k]:"Invalid family"; }
const char *hg_module(hg_kind k) { return (unsigned)k<=HG_RING?modules[k]:"invalid"; }
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
static void guard(hg_document *d,const char *condition)
{
    emit(d,"// synthesis translate_off\ninitial begin\n  if (%s) begin\n    $display(\"ECEN21 parameter error\");\n    $finish;\n  end\nend\n// synthesis translate_on\n",condition);
}
static void always_begin(const hg_config *c,hg_document *d,const char *reset_expression)
{
    emit(d,"always @(%s clk",c->falling?"negedge":"posedge");
    if(c->reset==HG_ASYNC_RESET)emit(d," or %s rst",c->reset_high?"posedge":"negedge");
    emit(d,") begin\n");
    if(c->reset!=HG_NO_RESET)emit(d,"  if (%srst) q <= %s;\n  else begin\n",c->reset_high?"":"!",reset_expression);
}
static void always_end(const hg_config *c,hg_document *d)
{ if(c->reset!=HG_NO_RESET)emit(d,"  end\n");emit(d,"end\nendmodule\n"); }
bool hg_generate(const hg_config *c,hg_document *d)
{
    if(!d)return false;
    memset(d,0,sizeof *d);
    if(!c)return reject(d,"Missing generator settings");
    if((unsigned)c->kind>HG_RING)return reject(d,"Invalid HDL family");
    if(c->width<1||c->width>32||c->slot>99)return reject(d,"Width1..32; export slot0..99");
    if((unsigned)c->reset>HG_ASYNC_RESET)return reject(d,"Invalid reset mode");
    if(c->kind==HG_ENCODER&&c->inputs!=4&&c->inputs!=8)return reject(d,"Encoder inputs must be4 or8");
    if(c->kind==HG_MODULO&&(c->modulus<1||c->modulus>65535||c->width<hg_min_width(c->modulus)||c->reset!=HG_SYNC_RESET))return reject(d,"Modulo1..65535; sufficient width; SYNC reset");
    if(c->kind==HG_RING&&(c->seed>=c->width||c->reset==HG_NO_RESET))return reject(d,"Ring seed<width; reset required");
    emit(d,"// ECEN21 Toolkit: %s\n// Verilog-2001. Pin order/priority are explicit below.\n",hg_name(c->kind));
    if(c->kind<=HG_STRUCTURAL) {
        const char *name=hg_module(c->kind);
        if(c->kind==HG_STRUCTURAL)emit(d,"module ecen21_fa1(input wire a,b,cin, output wire sum,cout);\nwire p;\nassign p=a^b;\nassign sum=p^cin;\nassign cout=(a&b)|(p&cin);\nendmodule\n\n");
        emit(d,"module %s #(parameter integer WIDTH=%u)(\n  input wire [WIDTH-1:0] a,b, input wire cin,\n  output wire [WIDTH-1:0] sum, output wire cout);\n",name,c->width);
        guard(d,"WIDTH<1 || WIDTH>32");
        if(c->kind==HG_BEHAVIORAL)emit(d,"assign {cout,sum}={1'b0,a}+{1'b0,b}+cin;\n");
        else emit(d,"// Bit0 is LSB; each stage consumes previous carry.\nwire [WIDTH:0] carry;\nassign carry[0]=cin;\nassign cout=carry[WIDTH];\ngenvar i;\ngenerate for(i=0;i<WIDTH;i=i+1) begin: bit_stage\n  ecen21_fa1 fa(.a(a[i]),.b(b[i]),.cin(carry[i]),\n    .sum(sum[i]),.cout(carry[i+1]));\nend endgenerate\n");
        emit(d,"endmodule\n");
    }
    else if(c->kind==HG_ENCODER) {
        emit(d,"// Ordinary active-high one-hot encoding. No priority.\n// Code ORs active indices even if invalid; only use when valid=1.\nmodule ecen21_encoder #(parameter integer INPUTS=%u)(\n  input wire [INPUTS-1:0] in,\n  output reg [(INPUTS==8?3:2)-1:0] code, output reg valid);\ninteger i,ones;\n",c->inputs);
        emit(d,"// Default INPUTS mapping from the shared encoder model:\n");
        for(unsigned i=0;i<c->inputs;i++) {
            bool valid;unsigned code=cb_encoder_code(1u<<i,c->inputs,&valid);
            if(!valid)return reject(d,"Shared encoder reference failed");
            emit(d,"// I%u alone -> code%u, valid1\n",i,code);
        }
        guard(d,"INPUTS!=4 && INPUTS!=8");
        emit(d,"always @* begin\n  code=0; ones=0;\n  for(i=0;i<INPUTS;i=i+1) begin\n    if(in[i]) begin code=code|i; ones=ones+1; end\n  end\n  valid=(ones==1);\nend\nendmodule\n");
    }
    else if(c->kind<=HG_JK) {
        const char *ports=c->kind==HG_D?"d":c->kind==HG_T?"t":"j,k";
        emit(d,"// q starts unspecified; reset/capture establishes state.\n// rst is ignored when reset mode is NONE.\nmodule %s #(parameter RESET_VALUE=1'b%u)(\n  input wire clk,rst,%s, output reg q);\n",hg_module(c->kind),c->reset_value?1:0,ports);
        guard(d,"RESET_VALUE !== 1'b0 && RESET_VALUE !== 1'b1");
        ff_type type=c->kind==HG_D?FF_D:c->kind==HG_T?FF_T:FF_JK;
        emit(d,"// Shared FF characteristic rows: Q A B -> next (active edge)\n");
        for(unsigned i=0;i<ff_characteristic_count(type);i++) {
            ff_transition tr;
            if(!ff_characteristic(type,i,&tr)||!tr.next_valid)return reject(d,"Shared FF reference failed");
            emit(d,"// %u %u %u -> %u\n",tr.q,tr.a,tr.b,tr.next_q);
        }
        always_begin(c,d,"RESET_VALUE");
        if(c->kind==HG_D)emit(d,"    q <= d;\n");
        if(c->kind==HG_T)emit(d,"    q <= q ^ t;\n");
        if(c->kind==HG_JK)emit(d,"    case ({j,k})\n      2'b00: q <= q;\n      2'b01: q <= 1'b0;\n      2'b10: q <= 1'b1;\n      2'b11: q <= ~q;\n      default: q <= 1'bx;\n    endcase\n");
        always_end(c,d);
    }
    else if(c->kind==HG_SHIFT) {
        emit(d,"// Priority: reset > load > right > left > hold.\n// Right inserts serial_left at MSB; left inserts serial_right at LSB.\n// q starts unspecified; reset or parallel load establishes state.\nmodule ecen21_shift #(parameter integer WIDTH=%u)(\n  input wire clk,rst,load,shift_right,shift_left,\n  input wire serial_left,serial_right,\n  input wire [WIDTH-1:0] parallel, output reg [WIDTH-1:0] q);\n",c->width);
        guard(d,"WIDTH<1 || WIDTH>32");
        always_begin(c,d,"{WIDTH{1'b0}}");
        emit(d,"    if(load) q <= parallel;\n    else if(shift_right)\n      q <= (q >> 1) | ({{(WIDTH-1){1'b0}},serial_left} << (WIDTH-1));\n    else if(shift_left)\n      q <= (q << 1) | {{(WIDTH-1){1'b0}},serial_right};\n    else q <= q;\n");always_end(c,d);
    }
    else if(c->kind==HG_MODULO) {
        emit(d,"// Synchronous reset. enable=0 holds; out-of-range states recover to0.\n// WIDTH defaults to safe ceil(log2(MODULUS)), with minimum1.\nmodule ecen21_modulo #(parameter integer MODULUS=%u,\n  parameter integer EXTRA_BITS=%u,\n  parameter integer WIDTH=EXTRA_BITS+(\n",c->modulus,c->width-hg_min_width(c->modulus));
        for(unsigned i=1;i<=15;i++)emit(d,"    (MODULUS<=%u)?%u:\n",1u<<i,i);
        emit(d,"    16))(input wire clk,rst,enable, output reg [WIDTH-1:0] q);\n");
        guard(d,"MODULUS<1 || MODULUS>65535 || EXTRA_BITS<0 || EXTRA_BITS>31 || WIDTH<1 || WIDTH>32 || (64'd1<<WIDTH)<MODULUS");
        emit(d,"// Selected calculator width=%u; override WIDTH if a wider counter is wanted.\n",c->width);
        always_begin(c,d,"{WIDTH{1'b0}}");emit(d,"    if(enable) begin\n      if(q>=MODULUS-1) q <= 0;\n      else q <= q+1'b1;\n    end\n");always_end(c,d);
    }
    else {
        emit(d,"// One-hot seed; enable holds/rotates. Reset establishes initial state.\nmodule ecen21_ring #(parameter integer WIDTH=%u,\n  parameter integer SEED_INDEX=%u)(\n  input wire clk,rst,enable, output reg [WIDTH-1:0] q);\nlocalparam [WIDTH-1:0] SEED={{(WIDTH-1){1'b0}},1'b1} << SEED_INDEX;\n",c->width,c->seed);
        guard(d,"WIDTH<1 || WIDTH>32 || SEED_INDEX<0 || SEED_INDEX>=WIDTH");
        always_begin(c,d,"SEED");
        emit(d,"    if(enable) q <= %s;\n",c->right?"(q >> 1) | (q << (WIDTH-1))":"(q << 1) | (q >> (WIDTH-1))");always_end(c,d);
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
    static const char *const prefix[]={"ECAB","ECAS","ECEN","ECDF","ECTF","ECJK","ECSR","ECMN","ECRG"};
    if(!c||!out||!cap||(unsigned)c->kind>HG_RING||c->slot>99){if(out&&cap)out[0]=0;return false;}
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
