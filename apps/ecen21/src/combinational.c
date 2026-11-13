/* Portable bounded combinational solvers. Reuses the verified exact Boolean
 * core/parser; preserves upstream applications and prior milestone engines. */
#include "combinational.h"
#include <stdio.h>
#include <string.h>

static unsigned pop(unsigned x) { unsigned n=0;for(;x;x>>=1)n+=x&1u;return n; }
static uint16_t domain(unsigned n) { return n==4?UINT16_MAX:(uint16_t)((1u<<(1u<<n))-1); }
const char *cb_name(cb_kind k)
{
    static const char *const names[]={"Half adder","Full adder / P4.12","3-input count / P3.21","Truth / equation -> circuit","Ordinary encoder","BCD seven-segment","Ripple-carry stages"};
    return k<=CB_RIPPLE?names[k]:"Invalid kind";
}
void cb_dirty(cb_model *m) { if(m){m->valid=false;m->verified=0;m->node_count=0;m->error[0]=0;} }
void cb_init(cb_model *m,cb_kind k)
{
    if(!m)return;
    memset(m,0,sizeof *m);m->kind=k;m->variables=k==CB_HA?2:3;
    m->encoder_inputs=4;m->width=4;m->a=7;m->b=1;
    m->select_count=2;m->select[0]=0;m->select[1]=1;m->select[2]=2;
    m->on=0x96;snprintf(m->expression,sizeof m->expression,"A+B");
}
unsigned cb_inputs(const cb_model *m)
{ return !m?0:m->kind==CB_ENCODER?m->encoder_inputs:m->kind==CB_DISPLAY?4:m->kind==CB_HA?2:m->kind==CB_FA||m->kind==CB_POPCOUNT?3:m->variables; }
unsigned cb_outputs(const cb_model *m)
{ return !m?0:m->kind==CB_ENCODER?(m->encoder_inputs==8?3:2):m->kind==CB_DISPLAY?7:m->kind==CB_HA||m->kind==CB_FA||m->kind==CB_POPCOUNT?2:1; }
void cb_input_name(const cb_model *m,unsigned i,char *text,size_t cap)
{
    if(!m||!text||!cap)return;
    if(m->kind==CB_ENCODER)snprintf(text,cap,"I%u",m->encoder_inputs-1-i);
    else if(m->kind==CB_DISPLAY)snprintf(text,cap,"B%u",3-i);
    else if(m->kind==CB_FA)snprintf(text,cap,"%s",i==0?"x0":i==1?"y0":"c0");
    else snprintf(text,cap,"%c",'A'+(int)i);
}
void cb_output_name(const cb_model *m,char *text,size_t cap)
{
    if(!m||!text||!cap)return;
    if(m->kind==CB_ENCODER)snprintf(text,cap,"Y%u",m->output);
    else if(m->kind==CB_DISPLAY)snprintf(text,cap,"%c",'a'+(int)m->output);
    else if(m->kind==CB_HA||m->kind==CB_FA)snprintf(text,cap,"%s",m->output?"Carry":"Sum");
    else if(m->kind==CB_POPCOUNT)snprintf(text,cap,"Count%u",m->output);
    else snprintf(text,cap,"F");
}
unsigned cb_encoder_code(unsigned p,unsigned n,bool *valid)
{
    unsigned code=0;
    if(valid)*valid=(n==4||n==8)&&p<(1u<<n)&&pop(p)==1;
    if(n!=4&&n!=8)return 0;
    for(unsigned i=0;i<n;i++)if(p&(1u<<i))code|=i;
    return code;
}
int cb_expected(const cb_model *m,unsigned row)
{
    if(!m)return -2;
    unsigned n=cb_inputs(m);
    if(n>8||row>=(1u<<n)||m->output>=cb_outputs(m))return -2;
    if(m->kind==CB_ENCODER)return (int)((cb_encoder_code(row,n,NULL)>>m->output)&1u);
    if(m->kind==CB_HA||m->kind==CB_FA||m->kind==CB_POPCOUNT) {
        unsigned sum=pop(row);return (int)((sum>>(m->output?1:0))&1u);
    }
    if(m->kind==CB_DISPLAY) {
        /* bit0=a, bit6=g; decimal glyphs, independent of electrical polarity. */
        static const unsigned glyph[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
        if(row>=10) return m->invalid_policy==2?-1:m->invalid_policy==0?0:(int)m->active_low;
        return (int)(((glyph[row]>>m->output)&1u)^(unsigned)m->active_low);
    }
    if(m->kind==CB_FUNCTION) {
        if(m->dc&(1u<<row))return -1;
        return (int)((m->on>>row)&1u);
    }
    return -2;
}
static bool fail(cb_model *m,const char *why)
{ cb_dirty(m);snprintf(m->error,sizeof m->error,"%s",why);return false; }
static uint16_t node(cb_model *m,cb_op op,unsigned a,unsigned b)
{
    if(op==CB_AND||op==CB_OR||op==CB_XOR) { if(a>b){unsigned t=a;a=b;b=t;} }
    for(unsigned i=0;i<m->node_count;i++)if(m->nodes[i].op==op&&m->nodes[i].a==a&&m->nodes[i].b==b)return (uint16_t)i;
    if(m->node_count==CB_NODES)return CB_NONE;
    unsigned i=m->node_count++;m->nodes[i]=(cb_node){op,(uint16_t)a,(uint16_t)b};return (uint16_t)i;
}
static uint16_t combine(cb_model *m,cb_op op,unsigned a,unsigned b)
{ return a==CB_NONE||b==CB_NONE?CB_NONE:node(m,op,a,b); }
static uint16_t gates(cb_model *m,const eb_result *r)
{
    unsigned out=node(m,r->form==EB_SOP?CB_ZERO:CB_ONE,0,0);
    for(unsigned i=0;i<r->count;i++) {
        unsigned term=node(m,r->form==EB_SOP?CB_ONE:CB_ZERO,0,0);
        for(unsigned j=0;j<r->variables;j++) {
            unsigned bit=1u<<(r->variables-1-j);
            if(!(r->cubes[i].fixed&bit))continue;
            unsigned x=node(m,CB_INPUT,j,0);
            bool positive=(r->cubes[i].bits&bit)!=0;
            if(r->form==EB_POS)positive=!positive;
            if(!positive)x=combine(m,CB_NOT,x,0);
            term=combine(m,r->form==EB_SOP?CB_AND:CB_OR,term,x);
        }
        out=combine(m,r->form==EB_SOP?CB_OR:CB_AND,out,term);
    }
    return (uint16_t)out;
}
static int eval_node(const cb_model *m,unsigned row,unsigned target)
{
    if(!m||cb_inputs(m)<2||cb_inputs(m)>8||m->node_count>CB_NODES||row>=(1u<<cb_inputs(m)))return -1;
    uint8_t values[CB_NODES];
    for(unsigned i=0;i<m->node_count;i++) {
        cb_node q=m->nodes[i];unsigned a=q.a,b=q.b;
        if(q.op>=CB_NOT&&(a>=i||(q.op!=CB_NOT&&b>=i)))return -1;
        switch(q.op) {
        case CB_ZERO:values[i]=0;break;case CB_ONE:values[i]=1;break;
        case CB_INPUT:if(a>=cb_inputs(m))return -1;values[i]=(uint8_t)((row>>(cb_inputs(m)-1-a))&1u);break;
        case CB_NOT:values[i]=!values[a];break;
        case CB_AND:values[i]=values[a]&values[b];break;
        case CB_OR:values[i]=values[a]|values[b];break;
        case CB_XOR:values[i]=values[a]^values[b];break;
        default:return -1;
        }
    }
    return target<m->node_count?values[target]:-1;
}
int cb_eval_gate(const cb_model *m,unsigned row,bool validity)
{ return !m?-1:eval_node(m,row,validity?m->valid_output:m->gate_output); }
int cb_eval_mux(const cb_model *m,unsigned row)
{
    if(!m||cb_inputs(m)<2||cb_inputs(m)>4||row>=(1u<<cb_inputs(m))||m->kind==CB_ENCODER||m->select_count<1||m->select_count>3)return -1;
    unsigned address=0,n=cb_inputs(m);
    for(unsigned i=0;i<m->select_count;i++) {
        if(m->select[i]>=n)return -1;
        address=(address<<1)|((row>>(n-1-m->select[i]))&1u);
    }
    return eval_node(m,row,m->mux_data[address]);
}
bool cb_ripple(unsigned width,unsigned a,unsigned b,unsigned cin,cb_stage stages[8],unsigned *sum,unsigned *cout)
{
    if(!stages||!sum||!cout||width<1||width>8||a>=(1u<<width)||b>=(1u<<width)||cin>1)return false;
    *sum=0;
    for(unsigned i=0;i<width;i++) {
        unsigned x=(a>>i)&1,y=(b>>i)&1;
        stages[i]=(cb_stage){x,y,cin,x^y^cin,(x&y)|(x&cin)|(y&cin)};
        *sum|=stages[i].sum<<i;cin=stages[i].cout;
    }
    *cout=cin;return true;
}
bool cb_solve(cb_model *m)
{
    if(!m)return false;
    cb_dirty(m);
    if(m->kind>CB_RIPPLE)return fail(m,"Invalid problem type");
    if(m->kind==CB_RIPPLE) {
        cb_stage stages[8];unsigned sum,cout;
        if(!cb_ripple(m->width,m->a,m->b,m->cin,stages,&sum,&cout))return fail(m,"Width1..8; operands must fit; Cin0/1");
        if(sum+(cout<<m->width)!=m->a+m->b+m->cin)return fail(m,"Carry-chain equivalence failed");
        m->verified=m->width;m->valid=true;return true;
    }
    unsigned n=cb_inputs(m);
    if(m->kind==CB_ENCODER) {
        if((n!=4&&n!=8)||m->output>=cb_outputs(m))return fail(m,"Encoder4/8 inputs; select valid output");
        unsigned code=node(m,CB_ZERO,0,0),any=code,pairs=code;
        for(unsigned i=0;i<n;i++) {
            unsigned x=node(m,CB_INPUT,n-1-i,0);any=combine(m,CB_OR,any,x);
            if((i>>m->output)&1)code=combine(m,CB_OR,code,x);
            for(unsigned j=0;j<i;j++)pairs=combine(m,CB_OR,pairs,combine(m,CB_AND,x,node(m,CB_INPUT,n-1-j,0)));
        }
        m->gate_output=(uint16_t)code;m->valid_output=combine(m,CB_AND,any,combine(m,CB_NOT,pairs,0));
    }
    else {
        if(n<2||n>4||m->output>=cb_outputs(m)||m->invalid_policy>2)return fail(m,"Invalid variables/output/policy");
        if(m->kind==CB_FUNCTION&&m->expression_source) {
            bs_model parsed;bs_init(&parsed);bs_set_variables(&parsed,n);
            if(!bs_set_expression(&parsed,m->expression))return fail(m,parsed.error);
            m->on=parsed.on;m->dc=0;
        }
        if(m->kind==CB_FUNCTION&&((m->on&m->dc)||((m->on|m->dc)&~domain(n))))return fail(m,"Truth/DC overlap or rows outside domain");
        m->truth=0;m->cares=0;
        for(unsigned row=0;row<(1u<<n);row++) {
            int y=cb_expected(m,row);if(y>=0){m->cares|=(uint16_t)(1u<<row);if(y)m->truth|=(uint16_t)(1u<<row);}
        }
        for(unsigned f=0;f<2;f++)if(eb_minimize(n,m->truth,(uint16_t)~m->cares&domain(n),(eb_form)f,&m->result[f]))return fail(m,"Minimization failed");
        if(m->form>EB_POS)return fail(m,"Invalid SOP/POS form");
        m->gate_output=gates(m,&m->result[m->form]);
        /* Fixed adder family also supplies an actual two-half-adder XOR path. */
        if(m->kind==CB_HA||m->kind==CB_FA||m->kind==CB_POPCOUNT) {
            unsigned a=node(m,CB_INPUT,0,0),b=node(m,CB_INPUT,1,0);
            unsigned p=combine(m,CB_XOR,a,b),carry=combine(m,CB_AND,a,b),sum=p;
            if(n==3) {
                unsigned c=node(m,CB_INPUT,2,0);
                sum=combine(m,CB_XOR,p,c);carry=combine(m,CB_OR,carry,combine(m,CB_AND,p,c));
            }
            m->gate_output=(uint16_t)(m->output?carry:sum);
        }
        /* Use the selected implementation's explicit DC completion. */
        if(m->select_count<1||m->select_count>3||m->select_count>n)return fail(m,"Select1..3 distinct variables");
        unsigned used=0;
        for(unsigned i=0;i<m->select_count;i++) {
            if(m->select[i]>=n||(used&(1u<<m->select[i])))return fail(m,"Select variables must be distinct/in range");
            used|=1u<<m->select[i];
        }
        for(unsigned d=0;d<(1u<<m->select_count);d++) {
            uint16_t truth=0;
            for(unsigned row=0;row<(1u<<n);row++) {
                unsigned source=row;
                for(unsigned i=0;i<m->select_count;i++) {
                    unsigned bit=1u<<(n-1-m->select[i]);source&=~bit;
                    if(d&(1u<<(m->select_count-1-i)))source|=bit;
                }
                if(eb_evaluate(&m->result[m->form],source))truth|=(uint16_t)(1u<<row);
            }
            if(eb_minimize(n,truth,0,EB_SOP,&m->cofactor[d]))return fail(m,"MUX cofactor failed");
            m->mux_data[d]=gates(m,&m->cofactor[d]);
        }
    }
    if(m->gate_output==CB_NONE||m->node_count>=CB_NODES)return fail(m,"Connection capacity exceeded");
    for(unsigned row=0;row<(1u<<n);row++) {
        int expected=cb_expected(m,row);
        if(expected>=0&&cb_eval_gate(m,row,false)!=expected)return fail(m,"Gate equivalence failed");
        if(m->kind!=CB_ENCODER&&cb_eval_mux(m,row)!=cb_eval_gate(m,row,false))return fail(m,"MUX completion equivalence failed");
        if(m->kind==CB_ENCODER&&cb_eval_gate(m,row,true)!=(pop(row)==1))return fail(m,"Validity circuit failed");
        if(expected>=0)m->verified++;
    }
    m->valid=true;return true;
}
unsigned cb_connection_lines(const cb_model *m)
{ return !m||!m->valid?0:m->kind==CB_RIPPLE?m->width+3:m->node_count+2+(m->kind==CB_ENCODER?1:m->select_count+(1u<<m->select_count)+1); }
bool cb_connection_line(const cb_model *m,unsigned line,char *text,size_t cap)
{
    if(!m||!text||!cap||line>=cb_connection_lines(m))return false;
    if(m->kind==CB_RIPPLE) {
        if(line==0)snprintf(text,cap,"FA0.Cin <- external Cin=%u",m->cin);
        else if(line<=m->width) {
            unsigned i=line-1;
            if(i==0)snprintf(text,cap,"FA0: x=A[0],y=B[0],Cin=external");
            else snprintf(text,cap,"FA%u: x=A[%u],y=B[%u],Cin=FA%u.Cout",i,i,i,i-1);
        }
        else if(line==m->width+1)snprintf(text,cap,"Result[i] <- FAi.Sum (LSB first)");
        else snprintf(text,cap,"CarryOut <- FA%u.Cout",m->width-1);
        return true;
    }
    char out[12];cb_output_name(m,out,sizeof out);
    if(line==0){snprintf(text,cap,"N refs local to %s; inputs MSB first",out);return true;}
    unsigned i=line-1;
    if(i<m->node_count) {
        cb_node q=m->nodes[i];char name[8];cb_input_name(m,q.a,name,sizeof name);
        static const char *const ops[]={"0","1","INPUT","NOT","AND","OR","XOR"};
        if(q.op<=CB_ONE)snprintf(text,cap,"N%u <- %s",i,ops[q.op]);
        else if(q.op==CB_INPUT)snprintf(text,cap,"N%u <- %s",i,name);
        else if(q.op==CB_NOT)snprintf(text,cap,"N%u <- NOT(N%u)",i,q.a);
        else snprintf(text,cap,"N%u <- %s(N%u,N%u)",i,ops[q.op],q.a,q.b);
        return true;
    }
    i-=m->node_count;
    if(i==0){snprintf(text,cap,"Gate %s <- N%u",out,m->gate_output);return true;}i--;
    if(m->kind==CB_ENCODER){snprintf(text,cap,"VALID <- N%u; code only valid if VALID=1",m->valid_output);return true;}
    if(i<m->select_count) {
        char name[8];cb_input_name(m,m->select[i],name,sizeof name);
        snprintf(text,cap,"MUX_%s.S%u <- %s",out,m->select_count-1-i,name);return true;
    }
    i-=m->select_count;
    if(i<(1u<<m->select_count))snprintf(text,cap,"MUX_%s.D%u <- N%u",out,i,m->mux_data[i]);
    else snprintf(text,cap,"MUX_%s.Out -> %s",out,out);
    return true;
}
