#include "timing.h"
#include <stdio.h>
#include <string.h>

static bool fail(tm_model *out,unsigned column,unsigned cell,const char *text)
{
    if(out) {
        out->valid=false;out->error_column=column;out->error_cell=cell;
        snprintf(out->error,sizeof out->error,"%s",text);
    }
    return false;
}
void tm_invalidate(tm_model *m)
{
    if(!m)return;
    m->valid=false;m->completed=0;m->error[0]='\0';
}
void tm_init(tm_model *m)
{
    if(!m)return;
    memset(m,0,sizeof *m);
    for(unsigned i=0;i<TM_CELLS;i++) {
        m->cells[i].type=TM_D;m->cells[i].a=1;m->cells[i].b=TM_ZERO;
        m->cells[i].clock=0;m->cells[i].edge=FF_RISING;m->cells[i].high=true;
        m->cells[i].reset=2;m->cells[i].reset_high=true;
    }
    m->cells[0].enabled=true;
    for(unsigned i=0;i<TM_GATES;i++)m->gates[i].op=TM_AND;
}
bool tm_set_cell(tm_model *m,unsigned cell,const tm_cell *value)
{
    if(!m)return false;
    tm_invalidate(m);
    if(cell>=TM_CELLS || !value)return fail(m,0,cell,"Invalid cell index/value");
    m->cells[cell]=*value;return true;
}
bool tm_set_gate(tm_model *m,unsigned gate,const tm_gate *value)
{
    if(!m)return false;
    tm_invalidate(m);
    if(gate>=TM_GATES || !value)return fail(m,0,gate,"Invalid gate index/value");
    m->gates[gate]=*value;return true;
}
bool tm_set_initial_inputs(tm_model *m,unsigned value)
{
    if(!m)return false;
    tm_invalidate(m);
    if(value>15)return fail(m,0,0,"Input word must be 0..15");
    m->initial_inputs=value;return true;
}
bool tm_set_event(tm_model *m,unsigned index,unsigned mask,unsigned values)
{
    if(!m)return false;
    tm_invalidate(m);
    if(index>=TM_EVENTS || mask>15 || values>15)return fail(m,index+1,0,"Invalid event index/mask/value");
    m->events[index].mask=(uint8_t)mask;m->events[index].values=(uint8_t)values;
    if(index>=m->event_count)m->event_count=index+1;
    return true;
}
bool tm_set_event_count(tm_model *m,unsigned count)
{
    if(!m)return false;
    tm_invalidate(m);
    if(count>TM_EVENTS)return fail(m,count,0,"At most 64 external events");
    m->event_count=count;return true;
}
static bool source_valid(const tm_model *m,unsigned source)
{
    if(source<TM_INPUTS || source==TM_ZERO || source==TM_ONE)return true;
    if(source<8)return m->cells[source-4].enabled;
    return source<12 && m->gates[source-8].enabled;
}
static bool validate(const tm_model *m,unsigned order[4],const char **error,unsigned *bad)
{
    if(!m){*error="Missing timing model";return false;}
    if(m->initial_inputs>15 || m->event_count>TM_EVENTS){*error="Invalid input word/event count";return false;}
    for(unsigned i=0;i<m->event_count;i++)
        if(m->events[i].mask>15 || m->events[i].values>15){*bad=i+1;*error="Invalid event word";return false;}
    for(unsigned i=0;i<TM_CELLS;i++) {
        const tm_cell *c=&m->cells[i];if(!c->enabled)continue;*bad=i;
        if(c->type<TM_SR || c->type>TM_SR_LATCH || c->initial>TM_X || c->reset_value>1 ||
            (c->edge!=FF_RISING && c->edge!=FF_FALLING) || c->reset_mode<TM_RESET_NONE || c->reset_mode>TM_RESET_ASYNC) {
            *error="Invalid cell type/initial/reset/edge";return false;
        }
        if(!source_valid(m,c->a) || !source_valid(m,c->clock) ||
            ((c->type==TM_SR || c->type==TM_JK || c->type==TM_SR_LATCH) && !source_valid(m,c->b)) ||
            (c->reset_mode!=TM_RESET_NONE && !source_valid(m,c->reset))) {
            *error="Cell references unavailable signal";return false;
        }
    }
    unsigned remaining=0;
    for(unsigned i=0;i<TM_GATES;i++) {
        const tm_gate *g=&m->gates[i];if(!g->enabled)continue;*bad=i;
        if(g->op<TM_NOT || g->op>TM_NAND || !source_valid(m,g->a) ||
            (g->op!=TM_NOT && !source_valid(m,g->b))){*error="Invalid gate/source";return false;}
        remaining|=1u<<i;
    }
    unsigned position=0;
    while(remaining) {
        bool progress=false;
        for(unsigned i=0;i<TM_GATES;i++)if(remaining&(1u<<i)) {
            const tm_gate *g=&m->gates[i];unsigned dependencies=0;
            if(g->a>=8 && g->a<12)dependencies|=1u<<(g->a-8);
            if(g->op!=TM_NOT && g->b>=8 && g->b<12)dependencies|=1u<<(g->b-8);
            if(dependencies&remaining)continue;
            order[position++]=i;remaining&=~(1u<<i);progress=true;
        }
        if(!progress){*error="Combinational gate cycle";return false;}
    }
    while(position<4)order[position++]=TM_GATES;
    return true;
}
unsigned tm_logic(tm_gate_op op,unsigned a,unsigned b)
{
    if(op<TM_NOT || op>TM_NAND || a>TM_X || (op!=TM_NOT && b>TM_X))return TM_X;
    if(op==TM_NOT)return a==TM_X?TM_X:a^1u;
    if(op==TM_AND || op==TM_NAND) {
        unsigned value=a==0 || b==0?0:a==1 && b==1?1:TM_X;
        return op==TM_NAND?(value==TM_X?TM_X:value^1u):value;
    }
    if(op==TM_OR)return a==1 || b==1?1:a==0 && b==0?0:TM_X;
    return a==TM_X || b==TM_X?TM_X:a^b;
}
static unsigned read_signal(const uint8_t signals[12],unsigned reference)
{
    return reference==TM_ZERO?0:reference==TM_ONE?1:signals[reference];
}
static void signals(const tm_model *m,const uint8_t q[4],unsigned inputs,const unsigned order[4],uint8_t values[12])
{
    for(unsigned i=0;i<4;i++){values[i]=(uint8_t)((inputs>>i)&1);values[4+i]=q[i];values[8+i]=TM_X;}
    for(unsigned j=0;j<4 && order[j]<4;j++) {
        unsigned i=order[j];const tm_gate *g=&m->gates[i];
        values[8+i]=(uint8_t)tm_logic(g->op,read_signal(values,g->a),g->op==TM_NOT?0:read_signal(values,g->b));
    }
}
static unsigned merge(unsigned a,unsigned b)
{
    return a==b?a:TM_X;
}
/* Set of concrete binary outcomes, expanded from abstract values. */
static unsigned active_value(tm_type type,unsigned q,unsigned a,unsigned b)
{
    unsigned outcomes=0;
    for(unsigned qq=0;qq<2;qq++)if(q==TM_X || q==qq)
    for(unsigned aa=0;aa<2;aa++)if(a==TM_X || a==aa)
    for(unsigned bb=0;bb<2;bb++)if(b==TM_X || b==bb) {
        ff_transition t;ff_evaluate((ff_type)(type==TM_SR_LATCH?TM_SR:type),qq,aa,bb,FF_RISING,FF_RISING,&t);
        outcomes|=t.next_valid?1u<<t.next_q:3u;
    }
    return outcomes==1?0:outcomes==2?1:TM_X;
}
/* 0=none,1=definite,2=possible edge. A stable abstract X is not an event. */
static unsigned clock_event(unsigned old,unsigned now,ff_event edge)
{
    if(old==now)return 0;
    unsigned before=edge==FF_RISING?0:1,after=before^1u;
    if(old==before && now==after)return 1;
    if((old==TM_X || old==before) && (now==TM_X || now==after))return 2;
    return 0;
}
static unsigned enabled_value(unsigned level,bool high)
{
    return level==TM_X?2:(unsigned)(level==(high?1u:0u));
}
static unsigned update(const tm_cell *c,unsigned q,const uint8_t before[12],const uint8_t current[12],bool initial,tm_reason *reason)
{
    *reason=initial?TM_INITIAL:TM_HOLD;
    bool latch=c->type==TM_LATCH || c->type==TM_SR_LATCH;
    unsigned fire=latch?enabled_value(read_signal(current,c->clock),c->high):
        initial?0:clock_event(read_signal(before,c->clock),read_signal(current,c->clock),c->edge);
    unsigned value=q;
    if(fire) {
        unsigned a=read_signal(current,c->a),b=(c->type==TM_SR || c->type==TM_JK || c->type==TM_SR_LATCH)?read_signal(current,c->b):0;
        value=c->type==TM_LATCH?a:active_value(c->type,q,a,b);
        if(c->reset_mode==TM_RESET_SYNC) {
            unsigned reset=enabled_value(read_signal(current,c->reset),c->reset_high);
            if(reset){value=reset==1?c->reset_value:merge(value,c->reset_value);*reason=reset==1?TM_SYNC_RESET:TM_UNKNOWN;}
        }
        if(*reason!=TM_SYNC_RESET && *reason!=TM_UNKNOWN) {
            if(latch)*reason=TM_TRANSPARENT;
            else if(fire==2)*reason=TM_CLOCK_UNCERTAIN;
            else if(c->type==TM_D)*reason=TM_CAPTURE;
            else if(c->type==TM_T)*reason=a==1?TM_TOGGLE:TM_HOLD;
            else if(a==1 && b==0)*reason=TM_SET;
            else if(a==0 && b==1)*reason=TM_CLEAR;
            else if(a==1 && b==1 && c->type==TM_JK)*reason=TM_TOGGLE;
        }
        if(fire==2) {
            value=merge(q,value);
            /* A known reset level cannot turn a possible clock/enable into
             * a definite activation, even when hold/reset values agree. */
            *reason=latch?TM_UNKNOWN:TM_CLOCK_UNCERTAIN;
        }
        if(value==TM_X && *reason!=TM_CLOCK_UNCERTAIN)*reason=TM_UNKNOWN;
    }
    if(c->reset_mode==TM_RESET_ASYNC) {
        unsigned reset=enabled_value(read_signal(current,c->reset),c->reset_high);
        if(reset) {
            value=reset==1?c->reset_value:merge(value,c->reset_value);
            *reason=reset==2 || value==TM_X?TM_UNKNOWN:TM_ASYNC_RESET;
        }
    }
    return value;
}
static bool settle(const tm_model *m,unsigned inputs,uint8_t q[4],uint8_t before[12],const unsigned order[4],bool initial,
    uint8_t current[12],uint8_t aggregate[4],unsigned *phases,tm_phase_log *log)
{
    signals(m,q,inputs,order,current);
    for(unsigned i=0;i<4;i++)aggregate[i]=(uint8_t)(initial?TM_INITIAL:TM_HOLD);
    if(log){log->count=1;memcpy(log->values[0],current,12);memcpy(log->reasons[0],aggregate,4);}
    for(unsigned phase=0;phase<TM_PHASES;phase++) {
        uint8_t next[4],why[4];bool changed=false;
        for(unsigned i=0;i<4;i++) {
            tm_reason reason=initial?TM_INITIAL:TM_HOLD;
            next[i]=m->cells[i].enabled?(uint8_t)update(&m->cells[i],q[i],before,current,initial,&reason):TM_X;
            why[i]=(uint8_t)reason;
            if(reason!=TM_HOLD && reason!=TM_INITIAL)aggregate[i]=(uint8_t)reason;
            if(next[i]!=q[i])changed=true;
        }
        *phases=phase+1;
        if(!changed) {
            if(log)for(unsigned i=0;i<4;i++)if(log->reasons[log->count-1][i]==TM_HOLD || log->reasons[log->count-1][i]==TM_INITIAL)
                log->reasons[log->count-1][i]=why[i];
            return true;
        }
        memcpy(before,current,12);memcpy(q,next,4);signals(m,q,inputs,order,current);
        if(log){unsigned index=log->count++;memcpy(log->values[index],current,12);memcpy(log->reasons[index],why,4);}
    }
    return false;
}
static bool simulate(const tm_model *m,tm_model *out,unsigned last,unsigned selected,tm_phase_log *log)
{
    unsigned order[4],bad=0;const char *error="Invalid timing model";
    if(log){memset(log,0,sizeof *log);log->column=selected;}
    if(!validate(m,order,&error,&bad))return fail(out,0,bad,error);
    if(last>m->event_count)return fail(out,last,0,"Event column out of range");
    uint8_t q[4],before[12],current[12],why[4];unsigned inputs=m->initial_inputs,phases=0;
    for(unsigned i=0;i<4;i++)q[i]=(uint8_t)(m->cells[i].enabled?m->cells[i].initial:TM_X);
    signals(m,q,inputs,order,before);
    if(!settle(m,inputs,q,before,order,true,current,why,&phases,selected==0?log:NULL))
        return fail(out,0,0,"No stable state within 32 settling phases");
    if(out){memcpy(out->values[0],current,12);memcpy(out->reasons[0],why,4);out->phases[0]=(uint8_t)phases;out->completed=1;}
    for(unsigned column=1;column<=last;column++) {
        memcpy(before,current,12);
        unsigned mask=m->events[column-1].mask;
        inputs=(inputs&~mask)|(m->events[column-1].values&mask);
        if(!settle(m,inputs,q,before,order,false,current,why,&phases,selected==column?log:NULL))
            return fail(out,column,0,"No stable state within 32 settling phases");
        if(out){memcpy(out->values[column],current,12);memcpy(out->reasons[column],why,4);out->phases[column]=(uint8_t)phases;out->completed=column+1;}
    }
    if(out){out->valid=true;out->error[0]='\0';}
    return true;
}
bool tm_run(tm_model *m)
{
    if(!m)return false;
    tm_invalidate(m);return simulate(m,m,m->event_count,TM_EVENTS+1,NULL);
}
bool tm_explain(const tm_model *m,unsigned column,tm_phase_log *log)
{
    if(!log || !m)return false;
    return simulate(m,NULL,column,column,log);
}
const char *tm_signal_name(unsigned signal)
{
    static const char *const names[]={"I0","I1","I2","I3","Q0","Q1","Q2","Q3","G0","G1","G2","G3","0","1"};
    return signal<14?names[signal]:"?";
}
const char *tm_type_name(tm_type type)
{
    static const char *const names[]={"SR FF","D FF","JK FF","T FF","D latch","SR latch"};
    return type>=TM_SR && type<=TM_SR_LATCH?names[type]:"?";
}
const char *tm_gate_name(tm_gate_op op)
{
    static const char *const names[]={"NOT","AND","OR","XOR","NAND"};
    return op>=TM_NOT && op<=TM_NAND?names[op]:"?";
}
const char *tm_reason_name(tm_reason reason)
{
    static const char *const names[]={"Initial","Hold","Capture D","Toggle","Set","Clear","Transparent","Sync reset","Async reset","Unknown/invalid","Possible edge"};
    return reason>=TM_INITIAL && reason<=TM_CLOCK_UNCERTAIN?names[reason]:"?";
}
