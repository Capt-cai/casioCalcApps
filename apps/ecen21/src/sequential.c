#include "sequential.h"
#include "solver.h"
#include <stdio.h>
#include <string.h>

static bool fail(sq_model *m, unsigned field, unsigned pos, const char *text)
{
    if(m) {
        m->valid=false; m->checked_rows=0; m->error_field=field; m->error_pos=pos;
        snprintf(m->error,sizeof m->error,"%s",text);
    }
    return false;
}
void sq_invalidate(sq_model *m)
{
    if(!m) return;
    m->valid=false; m->checked_rows=0; m->error[0]='\0';
}
void sq_init(sq_model *m)
{
    if(!m) return;
    memset(m,0,sizeof *m); m->bits=3; m->active_edge=FF_RISING;
    for(unsigned bit=0;bit<3;bit++) m->types[bit]=FF_T;
    strcpy(m->cycle_text[0],"03570"); strcpy(m->cycle_text[1],"01320");
    for(unsigned i=0;i<6;i++) strcpy(m->equations[i],"0");
}
static bool valid_spec(sq_model *m)
{
    if(!m || m->bits<2 || m->bits>3) return fail(m,11,0,"State bits must be 2 or 3");
    if((m->policy!=SQ_RETURN && m->policy!=SQ_HOLD && m->policy!=SQ_DC) || (m->active_edge!=FF_RISING && m->active_edge!=FF_FALLING))
        return fail(m,11,0,"Invalid policy or active edge");
    if(m->initial>=(1u<<m->bits) || m->recovery>=(1u<<m->bits) || m->fixed_e>1)
        return fail(m,11,0,"Initial/recovery/control out of range");
    for(unsigned i=0;i<m->bits;i++)
        if(m->types[i]!=FF_SR && m->types[i]!=FF_D && m->types[i]!=FF_JK && m->types[i]!=FF_T) return fail(m,11,i,"Invalid flip-flop type");
    m->variables=m->bits+(m->controlled?1u:0u); m->count=1u<<m->variables;
    return true;
}
bool sq_set_bits(sq_model *m,unsigned bits)
{
    if(!m) return false;
    sq_invalidate(m);
    if(bits<2 || bits>3) return fail(m,11,0,"State bits must be 2 or 3");
    m->bits=bits; return true;
}
bool sq_set_controlled(sq_model *m,bool value)
{
    if(!m) { return false; }
    sq_invalidate(m);
    m->controlled=value;
    return true;
}
bool sq_set_reverse(sq_model *m,bool value)
{
    if(!m) { return false; }
    sq_invalidate(m);
    m->reverse=value;
    return true;
}
bool sq_set_type(sq_model *m,unsigned bit,ff_type type)
{
    if(!m) { return false; }
    sq_invalidate(m);
    if(bit>=m->bits || (type!=FF_SR && type!=FF_D && type!=FF_JK && type!=FF_T)) { return fail(m,11,bit,"Invalid bit/type"); }
    m->types[bit]=type;return true;
}
bool sq_set_edge(sq_model *m,ff_event edge)
{
    if(!m) { return false; }
    sq_invalidate(m);
    if(edge!=FF_RISING && edge!=FF_FALLING) { return fail(m,11,0,"Choose rising/falling edge"); }
    m->active_edge=edge;return true;
}
bool sq_set_policy(sq_model *m,sq_policy policy)
{
    if(!m) { return false; }
    sq_invalidate(m);
    if(policy!=SQ_RETURN && policy!=SQ_HOLD && policy!=SQ_DC) { return fail(m,11,0,"Invalid unused-state policy"); }
    m->policy=policy;return true;
}
bool sq_set_initial(sq_model *m,unsigned value)
{
    if(!m) { return false; }
    sq_invalidate(m);
    if(m->bits<2 || m->bits>3 || value>=(1u<<m->bits)) { return fail(m,11,0,"Initial Q out of range"); }
    m->initial=value;return true;
}
bool sq_set_recovery(sq_model *m,unsigned value)
{
    if(!m) { return false; }
    sq_invalidate(m);
    if(m->bits<2 || m->bits>3 || value>=(1u<<m->bits)) { return fail(m,11,0,"Recovery Q out of range"); }
    m->recovery=value;return true;
}
bool sq_set_fixed_e(sq_model *m,unsigned e)
{
    if(!m) { return false; }
    sq_invalidate(m);
    if(e>1) { return fail(m,11,0,"E must be 0 or 1"); }
    m->fixed_e=e;return true;
}
static char *text_buffer(sq_model *m,unsigned field,size_t *cap)
{
    if(!m) { return NULL; }
    if(field<2){*cap=sizeof m->cycle_text[0];return m->cycle_text[field];}
    if(field<8){*cap=sizeof m->equations[0];return m->equations[field-2];}
    if(field<10){*cap=sizeof m->outputs[0];return m->outputs[field-8];}
    if(field==10){*cap=sizeof m->timeline;return m->timeline;}
    return NULL;
}
const char *sq_text(const sq_model *m,unsigned field)
{
    if(!m) { return NULL; }
    if(field<2) { return m->cycle_text[field]; }
    if(field<8) { return m->equations[field-2]; }
    if(field<10) { return m->outputs[field-8]; }
    return field==10?m->timeline:NULL;
}
bool sq_set_text(sq_model *m,unsigned field,const char *text)
{
    size_t cap=0;char *dest=text_buffer(m,field,&cap);
    if(!m) { return false; }
    sq_invalidate(m);
    if(!dest || !text) { return fail(m,field,0,"Invalid text field"); }
    size_t n=strlen(text);
    if(n>=cap){m->invalid_text_fields|=(uint16_t)(1u<<field);return fail(m,field,(unsigned)cap-1,"Text exceeds field capacity");}
    memcpy(dest,text,n+1);m->invalid_text_fields&=(uint16_t)~(1u<<field);return true;
}
static void pins_init(sq_model *m)
{
    m->pin_count=0;memset(m->pins,0,sizeof m->pins);
    for(unsigned b=m->bits;b-->0;) {
        unsigned count=m->types[b]==FF_JK || m->types[b]==FF_SR?2:1;
        for(unsigned input=0;input<count;input++) {
            sq_pin *p=&m->pins[m->pin_count++];p->bit=b;p->input=input;
            snprintf(p->name,sizeof p->name,"%s%u",ff_input_name(m->types[b],input),b);
        }
    }
}
static bool expression_mask(sq_model *m,unsigned field,const char *text,uint16_t *out)
{
    /* Reuse the accepted expression parser. Local bounded scratch permits
     * independent callers without mutable static parser state. */
    bs_model parser;bs_init(&parser);bs_set_variables(&parser,m->variables);
    if(!bs_set_expression(&parser,text)) { return fail(m,field,parser.error_pos,parser.error); }
    *out=parser.on;return true;
}
static bool outputs_parse(sq_model *m)
{
    memset(m->output_mask,0,sizeof m->output_mask);
    for(unsigned z=0;z<2;z++) {
        m->output_present[z]=m->outputs[z][0]!='\0';
        if(m->output_present[z] && !expression_mask(m,8+z,m->outputs[z],&m->output_mask[z])) { return false; }
    }
    return true;
}
static unsigned popcount(unsigned x)
{ unsigned count=0;for(;x;x>>=1)count+=x&1u;return count; }
static void parity_find(sq_model *m,sq_pin *p,unsigned form)
{
    p->parity_mask[form]=0;p->parity_invert[form]=false;
    for(unsigned mask=1;mask<(1u<<m->variables);mask++)
    for(unsigned inv=0;inv<2;inv++) {
        bool same=true;
        for(unsigned row=0;row<m->count;row++)
            if((int)((popcount(mask&row)&1u)^inv)!=eb_evaluate(&p->solution[form],row)){same=false;break;}
        if(same){p->parity_mask[form]=mask;p->parity_invert[form]=inv!=0;return;}
    }
}
static unsigned evaluate_vector(const sq_model *m,unsigned row,unsigned form)
{
    unsigned a[3]={0,0,0},b[3]={0,0,0},next=0;
    for(unsigned i=0;i<m->pin_count;i++) {
        const sq_pin *p=&m->pins[i];
        unsigned value=(unsigned)eb_evaluate(&p->solution[form],row);
        if(p->input)b[p->bit]=value;else a[p->bit]=value;
    }
    for(unsigned bit=0;bit<m->bits;bit++) {
        ff_transition t;
        ff_evaluate(m->types[bit],(m->rows[row].q>>bit)&1u,a[bit],b[bit],m->active_edge,m->active_edge,&t);
        if(!t.next_valid) { return SQ_UNDEFINED; }
        next|=t.next_q<<bit;
    }
    return next;
}
static bool minimize_and_verify(sq_model *m)
{
    for(unsigned i=0;i<m->pin_count;i++)for(unsigned form=0;form<2;form++) {
        sq_pin *p=&m->pins[i];
        if(eb_minimize(m->variables,p->on,p->dc,(eb_form)form,&p->solution[form]))
            return fail(m,11,i,"Boolean minimization failed");
        parity_find(m,p,form);
    }
    for(unsigned row=0;row<m->count;row++)for(unsigned form=0;form<2;form++) {
        unsigned next=evaluate_vector(m,row,form);m->rows[row].next[form]=next;
        if(m->rows[row].care && next!=m->rows[row].target)
            return fail(m,11,row,"Equation replay differs from state table");
        if(m->rows[row].care)m->checked_rows++;
    }
    if(!outputs_parse(m)) { return false; }
    m->valid=true;m->error[0]='\0';return true;
}
bool sq_synthesize_table(sq_model *m,const unsigned targets[16],uint16_t used,uint16_t dc)
{
    if(!m || !targets) { return fail(m,11,0,"Missing state table"); }
    sq_invalidate(m);if(!valid_spec(m)) { return false; }pins_init(m);
    uint32_t domain=(UINT32_C(1)<<m->count)-1u;
    if((used&dc) || (((uint32_t)used|dc)&~domain)) { return fail(m,11,0,"State masks exceed domain"); }
    for(unsigned row=0;row<m->count;row++) {
        sq_row *r=&m->rows[row];memset(r,0,sizeof *r);
        r->q=row>>(m->controlled?1:0);r->e=m->controlled?(row&1u):0;
        r->used=((used>>row)&1u)!=0;r->care=((dc>>row)&1u)==0;
        r->target=r->care?targets[row]:SQ_UNDEFINED;
        if(r->care && targets[row]>=(1u<<m->bits)) { return fail(m,11,row,"Next state out of range"); }
        for(unsigned i=0;i<m->pin_count;i++) {
            sq_pin *p=&m->pins[i];unsigned value=FF_X;
            if(r->care) {
                ff_excitation e;ff_get_excitation(m->types[p->bit],(r->q>>p->bit)&1u,(targets[row]>>p->bit)&1u,&e);
                value=p->input?e.b:e.a;
            }
            r->excitation[i]=value;
            if(value==FF_X)p->dc|=(uint16_t)(1u<<row);
            else if(value)p->on|=(uint16_t)(1u<<row);
        }
    }
    return minimize_and_verify(m);
}
static bool cycle_parse(sq_model *m,unsigned branch,unsigned states[9],unsigned *count)
{
    const char *text=m->cycle_text[branch];unsigned n=0,positions[9];
    for(unsigned i=0;text[i];i++) {
        char c=text[i];
        if(c==' ' || c==',' || c=='-' || c=='>')continue;
        if(c<'0'||c>'7') { return fail(m,branch,i,"Cycle needs decimal state digits0-7"); }
        if(n>=9) { return fail(m,branch,i,"Cycle has too many states"); }
        unsigned value=(unsigned)(c-'0');
        if(m->bits<2 || m->bits>3 || value>=(1u<<m->bits)) { return fail(m,branch,i,"Cycle state out of range"); }
        positions[n]=i;states[n++]=value;
    }
    if(n>1 && states[n-1]==states[0])n--;
    if(n<2) { return fail(m,branch,0,"Cycle needs at least two distinct states"); }
    unsigned seen=0;
    for(unsigned i=0;i<n;i++) {
        if(seen&(1u<<states[i])) { return fail(m,branch,positions[i],"Repeated internal cycle state"); }
        seen|=1u<<states[i];
    }
    *count=n;return true;
}
bool sq_solve(sq_model *m)
{
    if(!m) { return false; }
    sq_invalidate(m);if(!valid_spec(m)) { return false; }
    uint16_t active_fields=(uint16_t)((1u<<8)|(1u<<9));
    if(m->controlled)active_fields|=1u<<10;
    if(m->reverse) {
        for(unsigned bit=0;bit<m->bits;bit++) {
            active_fields|=(uint16_t)(1u<<(2+bit*2));
            if(m->types[bit]==FF_JK || m->types[bit]==FF_SR)active_fields|=(uint16_t)(1u<<(3+bit*2));
        }
    } else active_fields|=(uint16_t)(m->controlled?3u:1u);
    unsigned invalid_fields=m->invalid_text_fields&active_fields;
    if(invalid_fields){unsigned field=0;while(!(invalid_fields&(1u<<field)))field++;return fail(m,field,0,"Repair rejected text field before solving");}
    if(m->controlled && m->timeline[0]) {
        for(unsigned i=0;m->timeline[i];i++)if(m->timeline[i]!='0' && m->timeline[i]!='1')
            return fail(m,10,i,"E timeline requires only0/1");
    }
    if(!m->reverse) {
        unsigned targets[16]={0};uint16_t used=0,dc=0;
        for(unsigned row=0;row<m->count;row++)targets[row]=m->policy==SQ_HOLD?row>>(m->controlled?1:0):m->recovery;
        for(unsigned branch=0;branch<(m->controlled?2u:1u);branch++) {
            unsigned states[9],count;
            if(!cycle_parse(m,branch,states,&count)) { return false; }
            for(unsigned i=0;i<count;i++) {
                unsigned row=(states[i]<<(m->controlled?1:0))|branch;
                used|=(uint16_t)(1u<<row);targets[row]=states[(i+1)%count];
            }
        }
        if(m->policy==SQ_DC)dc=(uint16_t)(~used&((UINT32_C(1)<<m->count)-1u));
        return sq_synthesize_table(m,targets,used,dc);
    }
    pins_init(m);
    for(unsigned i=0;i<m->pin_count;i++) {
        sq_pin *p=&m->pins[i];unsigned field=2+p->bit*2+p->input;
        if(!expression_mask(m,field,m->equations[field-2],&p->on)) { return false; }
    }
    for(unsigned row=0;row<m->count;row++) {
        sq_row *r=&m->rows[row];memset(r,0,sizeof *r);r->q=row>>(m->controlled?1:0);r->e=m->controlled?row&1u:0;r->used=true;
    }
    /* Raw equations are fully specified. Minimize equivalent views, then
     * evaluate all pins from each same oldQ; invalidSR remains an invalid row. */
    if(!minimize_and_verify(m)) { return false; }
    for(unsigned row=0;row<m->count;row++) {
        sq_row *r=&m->rows[row];r->target=r->next[0];r->care=r->target!=SQ_UNDEFINED;
        for(unsigned i=0;i<m->pin_count;i++) {
            sq_pin *p=&m->pins[i];unsigned value=FF_X;
            if(r->care){ff_excitation e;ff_get_excitation(m->types[p->bit],(r->q>>p->bit)&1u,(r->target>>p->bit)&1u,&e);value=p->input?e.b:e.a;}
            r->excitation[i]=value;
        }
        if(r->next[0]!=r->next[1]) { return fail(m,11,row,"Equivalent reverse forms disagree"); }
        if(r->care)m->checked_rows+=2;
    }
    return true;
}
int sq_pin_value(const sq_model *m,eb_form form,unsigned pin,unsigned row)
{
    if(!m || !m->valid || (form!=EB_SOP && form!=EB_POS) || pin>=m->pin_count || row>=m->count) { return -1; }
    return eb_evaluate(&m->pins[pin].solution[form],row);
}
int sq_format_equation(const sq_model *m,eb_form form,unsigned pin,char *buffer,size_t capacity)
{
    if(!m || !m->valid || (form!=EB_SOP && form!=EB_POS) || pin>=m->pin_count) { return -1; }
    return eb_format(&m->pins[pin].solution[form],buffer,capacity);
}
int sq_output_value(const sq_model *m,unsigned output,unsigned row)
{
    if(!m || !m->valid || output>=2 || row>=m->count || !m->output_present[output]) { return -1; }
    return (m->output_mask[output]>>row)&1u;
}
bool sq_next(const sq_model *m,eb_form form,unsigned q,unsigned e,ff_event event,unsigned *next)
{
    if(!m || !m->valid || !next || (form!=EB_SOP && form!=EB_POS) || q>=(1u<<m->bits) || e>1 || (event!=FF_NONE && event!=FF_RISING && event!=FF_FALLING)) { return false; }
    if(event!=m->active_edge){*next=q;return true;}
    unsigned row=(q<<(m->controlled?1:0))|(m->controlled?e:0);
    if(m->rows[row].next[form]==SQ_UNDEFINED) { return false; }
    *next=m->rows[row].next[form];return true;
}
bool sq_trace(const sq_model *m,eb_form form,unsigned start,unsigned e,const char *timeline,sq_trace_result *out)
{
    if(!m || !m->valid || !out || (form!=EB_SOP && form!=EB_POS) || start>=(1u<<m->bits) || e>1) { return false; }
    size_t length=m->controlled && timeline?strlen(timeline):0;
    if(length>SQ_MAX_TRACE) { return false; }
    for(unsigned i=0;i<length;i++)if(timeline[i]!='0' && timeline[i]!='1') { return false; }
    memset(out,0,sizeof *out);out->states[0]=start;
    unsigned limit=length?(unsigned)length:(1u<<m->bits);
    for(unsigned i=0;i<limit;i++) {
        unsigned control=length?(unsigned)(timeline[i]-'0'):e,next;
        out->controls[i]=control;
        if(!sq_next(m,form,out->states[i],control,m->active_edge,&next)){out->invalid=true;return true;}
        out->states[++out->steps]=next;
        if(!length)for(unsigned j=0;j<out->steps;j++)if(out->states[j]==next) {
            out->cycle=true;out->prefix=j;out->period=out->steps-j;return true;
        }
    }
    out->limited=true;return true;
}
const char *sq_variable_name(const sq_model *m,unsigned variable)
{
    static const char *const q[]={"Q0","Q1","Q2"};
    if(!m || m->bits<2 || m->bits>3 || variable>=m->bits+(m->controlled?1u:0u)) { return "?"; }
    return variable<m->bits?q[m->bits-1-variable]:"E";
}
const char *sq_policy_name(sq_policy policy)
{ return policy==SQ_RETURN?"RETURN":policy==SQ_HOLD?"HOLD":policy==SQ_DC?"DC":"?"; }
