#include "timing.h"
#include "timing_checks.h"
#include <string.h>

#define CHECK(x) do { ++*checks; if(!(x)) return __LINE__; } while(0)
static unsigned concrete(tm_type type,unsigned q,unsigned a,unsigned b)
{
    static const unsigned truth[4][8]={{0,0,1,2,1,0,1,2},{0,0,1,1,0,0,1,1},
        {0,0,1,1,1,0,1,0},{0,0,1,1,1,1,0,0}};
    if(type==TM_LATCH)return a;
    return truth[type==TM_SR_LATCH?TM_SR:type][q*4+a*2+b];
}
static unsigned abstract(tm_type type,unsigned q,unsigned a,unsigned b)
{
    unsigned value=3;
    for(unsigned qq=0;qq<2;qq++)if(q==2 || qq==q)
    for(unsigned aa=0;aa<2;aa++)if(a==2 || aa==a)
    for(unsigned bb=0;bb<2;bb++)if(b==2 || bb==b) {
        unsigned next=concrete(type,qq,aa,bb);
        if(value==3)value=next;
        else if(value!=next)value=2;
    }
    return value;
}
static void cell(tm_model *m,unsigned index,tm_type type,unsigned a,unsigned b,unsigned clock,unsigned initial)
{
    tm_cell c=m->cells[index];c.enabled=true;c.type=type;c.a=a;c.b=b;c.clock=clock;c.initial=initial;
    tm_set_cell(m,index,&c);
}
static void gate(tm_model *m,unsigned index,tm_gate_op op,unsigned a,unsigned b)
{
    tm_gate g={true,op,a,b};tm_set_gate(m,index,&g);
}
static int local_devices(unsigned *checks)
{
    /* Exhaustive binary local device/edge/reset combinations use a literal
     * truth-table oracle, not production ff_evaluate. */
    for(unsigned type=TM_SR;type<=TM_SR_LATCH;type++)
    for(unsigned q=0;q<2;q++)for(unsigned a=0;a<2;a++)for(unsigned b=0;b<2;b++)
    for(unsigned old=0;old<2;old++)for(unsigned now=0;now<2;now++)
    for(unsigned polarity=0;polarity<2;polarity++) {
        tm_model m;tm_init(&m);cell(&m,0,(tm_type)type,1,2,0,q);
        m.cells[0].edge=polarity?FF_RISING:FF_FALLING;m.cells[0].high=polarity!=0;
        m.initial_inputs=old|(a<<1)|(b<<2);
        CHECK(tm_set_event(&m,0,1,now));CHECK(tm_run(&m));
        unsigned initial=q;
        bool latch=type==TM_LATCH || type==TM_SR_LATCH;
        if(latch && old==polarity)initial=concrete((tm_type)type,q,a,b);
        unsigned expected=initial;
        if(latch && now==polarity)expected=abstract((tm_type)type,initial,a,b);
        else if(!latch && old!=now && now==polarity)expected=concrete((tm_type)type,q,a,b);
        CHECK(m.values[0][4]==initial);CHECK(m.values[1][4]==expected);
        CHECK(m.completed==2 && m.valid);
        tm_phase_log log;CHECK(tm_explain(&m,1,&log));CHECK(log.column==1 && log.count>=1 && log.count<=33);
        for(unsigned s=0;s<12;s++)CHECK(log.values[log.count-1][s]==m.values[1][s]);
        uint8_t before[12];memcpy(before,m.values[1],12);
        CHECK(tm_set_event(&m,1,0,15));CHECK(tm_run(&m));
        for(unsigned s=0;s<12;s++)CHECK(m.values[2][s]==before[s]);
    }
    for(unsigned type=TM_SR;type<=TM_SR_LATCH;type++)
    for(unsigned q=0;q<2;q++)for(unsigned reset_mode=TM_RESET_SYNC;reset_mode<=TM_RESET_ASYNC;reset_mode++)
    for(unsigned reset_high=0;reset_high<2;reset_high++)for(unsigned reset_value=0;reset_value<2;reset_value++)
    for(unsigned fire=0;fire<2;fire++) {
        tm_model m;tm_init(&m);cell(&m,0,(tm_type)type,TM_ONE,TM_ONE,0,q);
        tm_cell *c=&m.cells[0];c->reset_mode=(tm_reset_mode)reset_mode;c->reset=2;
        c->reset_high=reset_high!=0;c->reset_value=reset_value;
        /* Latch begins closed; all others begin low clock. */
        m.initial_inputs=(reset_high?0:4);
        unsigned event=(reset_high?4:0)|(fire?1:0);
        CHECK(tm_set_event(&m,0,5,event));CHECK(tm_run(&m));
        unsigned expected=reset_mode==TM_RESET_ASYNC || fire?reset_value:q;
        CHECK(m.values[1][4]==expected);
        if(reset_mode==TM_RESET_ASYNC || fire)CHECK(m.reasons[1][0]==(reset_mode==TM_RESET_ASYNC?TM_ASYNC_RESET:TM_SYNC_RESET));
    }
    return 0;
}
static int gates_unknown(unsigned *checks)
{
    /* Independent expansion of3-value gate inputs. */
    for(unsigned op=TM_NOT;op<=TM_NAND;op++)for(unsigned a=0;a<3;a++)for(unsigned b=0;b<3;b++) {
        unsigned expected=3;
        for(unsigned aa=0;aa<2;aa++)if(a==2 || a==aa)
        for(unsigned bb=0;bb<2;bb++)if(b==2 || b==bb) {
            unsigned value=op==TM_NOT?!aa:op==TM_AND?(aa&&bb):op==TM_OR?(aa||bb):op==TM_XOR?(aa!=bb):!(aa&&bb);
            expected=expected==3?value:expected==value?expected:2;
        }
        CHECK(tm_logic((tm_gate_op)op,a,b)==expected);
    }
    for(unsigned type=TM_SR;type<=TM_SR_LATCH;type++)
    for(unsigned q=0;q<3;q++)for(unsigned a=0;a<3;a++)for(unsigned b=0;b<3;b++) {
        tm_model m;tm_init(&m);
        /* Unknown a/b are provided by disabled-input-free cells that hold X;
         * q states are explicit and source clocks are constant. */
        cell(&m,1,TM_D,TM_ZERO,TM_ZERO,TM_ZERO,a);
        cell(&m,2,TM_D,TM_ZERO,TM_ZERO,TM_ZERO,b);
        cell(&m,0,(tm_type)type,5,6,0,q);
        CHECK(tm_set_event(&m,0,1,1));CHECK(tm_run(&m));
        CHECK(m.values[1][4]==abstract((tm_type)type,q,a,b));
    }
    tm_model m;tm_init(&m);cell(&m,1,TM_D,1,TM_ZERO,0,TM_X);
    /* Q1 is initially X, next captures1 and creates X->1 possible clock. */
    cell(&m,0,TM_T,TM_ONE,TM_ZERO,5,0);m.initial_inputs=2;
    CHECK(tm_set_event(&m,0,0,0));CHECK(tm_set_event(&m,1,2,0));
    CHECK(tm_set_event(&m,2,3,3));CHECK(tm_run(&m));
    CHECK(m.values[1][4]==0 && m.values[2][4]==0);
    CHECK(m.values[3][5]==1 && m.values[3][4]==TM_X);
    CHECK(m.reasons[3][0]==TM_CLOCK_UNCERTAIN);
    /* A definite later D capture recovers an unknown current state. */
    tm_init(&m);m.cells[0].initial=TM_X;m.initial_inputs=2;
    CHECK(tm_set_event(&m,0,1,1));CHECK(tm_run(&m));CHECK(m.values[1][4]==1);
    return 0;
}
static int clock_and_reset_boundaries(unsigned *checks)
{
    for(unsigned type=TM_SR;type<=TM_T;type++)
    for(unsigned q=0;q<3;q++)for(unsigned old=0;old<3;old++)for(unsigned now=0;now<3;now++)
    for(unsigned edge=FF_RISING;edge<=FF_FALLING;edge++) {
        tm_model m;tm_init(&m);cell(&m,2,TM_D,TM_ZERO,TM_ZERO,TM_ZERO,now);
        cell(&m,1,TM_D,6,TM_ZERO,0,old);
        cell(&m,0,(tm_type)type,TM_ONE,TM_ZERO,5,q);m.cells[0].edge=(ff_event)edge;
        CHECK(tm_set_event(&m,0,1,1));CHECK(tm_run(&m));
        unsigned expected=q;
        if(old!=now) {
            bool hold=false,capture=false;
            for(unsigned a=0;a<2;a++)if(old==2 || old==a)
            for(unsigned b=0;b<2;b++)if(now==2 || now==b) {
                if(a!=b && b==(edge==FF_RISING?1u:0u))capture=true;
                else hold=true;
            }
            if(capture) {
                unsigned next=abstract((tm_type)type,q,1,0);
                expected=hold && next!=q?TM_X:next;
            }
        }
        CHECK(m.values[1][4]==expected);
        CHECK(m.values[1][5]==now);
    }
    for(unsigned mode=TM_RESET_SYNC;mode<=TM_RESET_ASYNC;mode++) {
        tm_model m;tm_init(&m);m.cells[0].initial=1;m.cells[0].a=TM_ONE;
        m.cells[0].reset_mode=(tm_reset_mode)mode;m.cells[0].reset=2;
        CHECK(tm_set_event(&m,0,4,4));CHECK(tm_set_event(&m,1,1,1));CHECK(tm_run(&m));
        CHECK(m.values[1][4]==(mode==TM_RESET_ASYNC?0u:1u));CHECK(m.values[2][4]==0);
        CHECK(m.reasons[2][0]==(mode==TM_RESET_ASYNC?TM_ASYNC_RESET:TM_SYNC_RESET));
    }
    /* Known output despite uncertain reset must not claim a definite reset. */
    tm_model m;tm_init(&m);cell(&m,1,TM_D,TM_ZERO,TM_ZERO,TM_ZERO,TM_X);
    m.cells[0].reset_mode=TM_RESET_ASYNC;m.cells[0].reset=5;
    CHECK(tm_run(&m));CHECK(m.values[0][4]==0 && m.reasons[0][0]==TM_UNKNOWN);
    /* Independent simultaneous swap and SR-latch invalid/recovery paths. */
    tm_init(&m);cell(&m,0,TM_D,5,TM_ZERO,0,1);cell(&m,1,TM_D,4,TM_ZERO,0,0);
    CHECK(tm_set_event(&m,0,1,1));CHECK(tm_run(&m));CHECK(m.values[1][4]==0 && m.values[1][5]==1);
    tm_init(&m);cell(&m,0,TM_SR_LATCH,1,2,0,0);m.initial_inputs=1;
    CHECK(tm_set_event(&m,0,6,6));CHECK(tm_set_event(&m,1,6,2));CHECK(tm_run(&m));
    CHECK(m.values[1][4]==TM_X && m.values[2][4]==1);
    /* A two-latch ring has no stable zero-delay binary assignment. */
    tm_init(&m);cell(&m,0,TM_LATCH,5,TM_ZERO,TM_ONE,0);cell(&m,1,TM_LATCH,8,TM_ZERO,TM_ONE,0);gate(&m,0,TM_NOT,4,0);
    CHECK(!tm_run(&m) && !m.valid);
    return 0;
}
static int possible_reset_diagnostics(unsigned *checks)
{
    /* Literal hold/reset outcome table indexed resetValue,currentQ. The
     * activating edge is only possible (X -> selected known clock level). */
    static const unsigned merged[2][2]={{0,TM_X},{TM_X,1}};
    for(unsigned edge=FF_RISING;edge<=FF_FALLING;edge++)
    for(unsigned reset=0;reset<2;reset++)for(unsigned q=0;q<2;q++) {
        tm_model m;tm_init(&m);
        cell(&m,1,TM_D,edge==FF_RISING?TM_ONE:TM_ZERO,TM_ZERO,0,TM_X);
        cell(&m,0,TM_D,reset?TM_ZERO:TM_ONE,TM_ZERO,5,q);
        m.cells[0].edge=(ff_event)edge;m.cells[0].reset_mode=TM_RESET_SYNC;
        m.cells[0].reset=TM_ONE;m.cells[0].reset_value=reset;
        CHECK(tm_set_event(&m,0,1,1));CHECK(tm_run(&m));
        CHECK(m.values[0][4]==q);
        CHECK(m.values[1][4]==merged[reset][q]);
        CHECK(m.reasons[1][0]==TM_CLOCK_UNCERTAIN);
        tm_phase_log log;CHECK(tm_explain(&m,1,&log));
        for(unsigned point=0;point<log.count;point++)CHECK(log.reasons[point][0]!=TM_SYNC_RESET);
        /* A definite asynchronous reset really is independent of clock
         * uncertainty and must still override both hold and capture. */
        m.cells[0].reset_mode=TM_RESET_ASYNC;tm_invalidate(&m);
        CHECK(tm_run(&m));CHECK(m.values[1][4]==reset);
        CHECK(m.reasons[1][0]==TM_ASYNC_RESET);
    }
    for(unsigned type=TM_LATCH;type<=TM_SR_LATCH;type++)
    for(unsigned reset=0;reset<2;reset++)for(unsigned q=0;q<2;q++) {
        tm_model m;tm_init(&m);cell(&m,1,TM_D,TM_ZERO,TM_ZERO,TM_ZERO,TM_X);
        cell(&m,0,(tm_type)type,TM_ONE,TM_ONE,5,q);
        m.cells[0].reset_mode=TM_RESET_SYNC;m.cells[0].reset=TM_ONE;
        m.cells[0].reset_value=reset;
        CHECK(tm_set_event(&m,0,0,0));CHECK(tm_run(&m));
        CHECK(m.values[0][4]==merged[reset][q]);
        CHECK(m.values[1][4]==merged[reset][q]);
        CHECK(m.reasons[0][0]==TM_UNKNOWN && m.reasons[1][0]==TM_UNKNOWN);
    }
    return 0;
}
static int review_fixtures(unsigned *checks)
{
    tm_model m;tm_init(&m);cell(&m,0,TM_LATCH,1,TM_ZERO,0,0);
    cell(&m,1,TM_LATCH,4,TM_ZERO,0,0);m.cells[1].high=false;
    const unsigned masks[]={1,2,1,2,1,1,2,1,1},values[]={1,2,0,0,1,0,2,1,0};
    const unsigned master[]={0,0,1,1,1,0,0,0,1,1},slave[]={0,0,0,1,1,1,0,0,0,1};
    for(unsigned i=0;i<9;i++)CHECK(tm_set_event(&m,i,masks[i],values[i]));
    CHECK(tm_run(&m));
    for(unsigned i=0;i<10;i++){CHECK(m.values[i][4]==master[i]);CHECK(m.values[i][5]==slave[i]);}
    /* Simultaneous falling clock and changed D must close master before slave
     * opens: slave sees old settled master, not newly changed D. */
    tm_init(&m);cell(&m,0,TM_LATCH,1,TM_ZERO,0,0);cell(&m,1,TM_LATCH,4,TM_ZERO,0,0);m.cells[1].high=false;
    m.initial_inputs=3;CHECK(tm_set_event(&m,0,3,0));CHECK(tm_run(&m));
    CHECK(m.values[0][4]==1 && m.values[0][5]==0);CHECK(m.values[1][4]==1 && m.values[1][5]==1);
    /* Review2 comparison: high latch tracks D inside high clock; rising and
     * falling devices sample only their own edges. */
    tm_init(&m);cell(&m,0,TM_LATCH,1,TM_ZERO,0,0);cell(&m,1,TM_D,1,TM_ZERO,0,0);cell(&m,2,TM_D,1,TM_ZERO,0,0);m.cells[2].edge=FF_FALLING;
    const unsigned words[]={2,3,1,3,2,0,2,3,1,0};
    const unsigned qa[]={0,0,1,0,1,1,1,1,1,0,0};
    const unsigned qb[]={0,0,1,1,1,1,1,1,1,1,1};
    const unsigned qc[]={0,0,0,0,0,1,1,1,1,1,0};
    for(unsigned i=0;i<10;i++){CHECK(tm_set_event(&m,i,3,words[i]));}
    CHECK(tm_run(&m));
    for(unsigned i=0;i<11;i++){CHECK(m.values[i][4]==qa[i]);CHECK(m.values[i][5]==qb[i]);CHECK(m.values[i][6]==qc[i]);}
    /* Review3 positive Q-derived clocks: conventional numeric Q2Q1Q0 counts
     * down after initial ripple000->111. Source's Q0-first labels differ. */
    tm_init(&m);for(unsigned i=0;i<3;i++)cell(&m,i,TM_T,TM_ONE,TM_ZERO,i?3+i:0,0);
    for(unsigned i=0;i<16;i++){CHECK(tm_set_event(&m,i,1,(i&1)?0:1));}
    CHECK(tm_run(&m));
    for(unsigned i=0;i<8;i++) {
        unsigned column=2*i+1,word=m.values[column][4]|(m.values[column][5]<<1)|(m.values[column][6]<<2);
        CHECK(word==7-i);
    }
    tm_phase_log log;CHECK(tm_explain(&m,1,&log));CHECK(log.count==4);
    CHECK(log.values[0][4]==0 && log.values[1][4]==1 && log.values[1][5]==0);
    CHECK(log.values[2][5]==1 && log.values[2][6]==0 && log.values[3][6]==1);
    /* Review4 four synchronous T stages: all equations see same oldQ. */
    tm_init(&m);for(unsigned i=0;i<4;i++)cell(&m,i,TM_T,i==0?TM_ONE:i==1?4:6+i,TM_ZERO,0,i<3?1:0);
    gate(&m,0,TM_AND,4,5);gate(&m,1,TM_AND,8,6);
    for(unsigned i=0;i<32;i++){CHECK(tm_set_event(&m,i,1,(i&1)?0:1));}
    CHECK(tm_run(&m));
    for(unsigned i=0;i<16;i++) {
        unsigned word=0;for(unsigned bit=0;bit<4;bit++)word|=m.values[2*i+1][4+bit]<<bit;
        CHECK(word==(i+8)%16);
    }
    /* Review5 feedback reset must use oldQ NAND at triggering edge. */
    tm_init(&m);for(unsigned i=0;i<3;i++){cell(&m,i,TM_T,i==0?TM_ONE:i==1?4:8,TM_ZERO,0,0);m.cells[i].reset_mode=TM_RESET_SYNC;m.cells[i].reset=9;m.cells[i].reset_high=false;}
    gate(&m,0,TM_AND,4,5);gate(&m,1,TM_NAND,4,6);
    for(unsigned i=0;i<16;i++){CHECK(tm_set_event(&m,i,1,(i&1)?0:1));}
    CHECK(tm_run(&m));
    const unsigned sequence[]={1,2,3,4,5,0,1,2};
    for(unsigned i=0;i<8;i++){unsigned c=2*i+1;CHECK((unsigned)(m.values[c][4]|(m.values[c][5]<<1)|(m.values[c][6]<<2))==sequence[i]);}
    for(unsigned bit=0;bit<3;bit++)CHECK(m.reasons[11][bit]==TM_SYNC_RESET);
    return 0;
}
static int boundaries(unsigned *checks)
{
    tm_model m;tm_init(&m);
    CHECK(tm_run(&m));CHECK(m.valid && m.completed==1);
    CHECK(tm_set_event_count(&m,64));for(unsigned i=0;i<64;i++)CHECK(tm_set_event(&m,i,3,i%4));
    CHECK(tm_run(&m));CHECK(m.completed==65);
    tm_phase_log log;CHECK(tm_explain(&m,64,&log));CHECK(log.count>=1);
    CHECK(!tm_explain(&m,65,&log));CHECK(!tm_explain(NULL,0,&log));CHECK(!tm_explain(&m,0,NULL));
    CHECK(!tm_set_event(&m,64,0,0) && !m.valid);CHECK(!tm_set_event_count(&m,65));
    CHECK(!tm_set_event(&m,0,16,0));CHECK(!tm_set_event(&m,0,0,16));
    CHECK(!tm_set_initial_inputs(&m,16));CHECK(!tm_set_cell(&m,4,&m.cells[0]));CHECK(!tm_set_gate(&m,4,&m.gates[0]));
    CHECK(!tm_set_cell(&m,0,NULL));CHECK(!tm_set_gate(&m,0,NULL));
    tm_init(NULL);tm_invalidate(NULL);CHECK(!tm_run(NULL));CHECK(!tm_set_event(NULL,0,0,0));CHECK(!tm_set_event_count(NULL,0));
    CHECK(!tm_set_initial_inputs(NULL,0));CHECK(!tm_set_cell(NULL,0,NULL));CHECK(!tm_set_gate(NULL,0,NULL));
    tm_init(&m);m.cells[0].a=14;CHECK(!tm_run(&m) && !m.valid);
    tm_init(&m);m.cells[0].a=5;CHECK(!tm_run(&m));
    tm_init(&m);m.cells[0].a=8;CHECK(!tm_run(&m));
    tm_init(&m);m.cells[0].type=(tm_type)-1;CHECK(!tm_run(&m));
    tm_init(&m);m.cells[0].edge=FF_NONE;CHECK(!tm_run(&m));
    tm_init(&m);m.cells[0].reset_mode=(tm_reset_mode)-1;CHECK(!tm_run(&m));
    tm_init(&m);m.cells[0].initial=3;CHECK(!tm_run(&m));
    tm_init(&m);m.event_count=65;CHECK(!tm_run(&m));
    tm_init(&m);m.initial_inputs=16;CHECK(!tm_run(&m));
    tm_init(&m);m.event_count=1;m.events[0].mask=16;CHECK(!tm_run(&m));
    tm_init(&m);gate(&m,0,TM_NOT,8,0);CHECK(!tm_run(&m) && strstr(m.error,"cycle"));
    tm_init(&m);gate(&m,0,TM_NOT,9,0);gate(&m,1,TM_NOT,8,0);CHECK(!tm_run(&m));
    /* Forward acyclic gate references are topologically resolved. */
    tm_init(&m);gate(&m,0,TM_NOT,9,0);gate(&m,1,TM_NOT,1,0);m.initial_inputs=2;CHECK(tm_run(&m));CHECK(m.values[0][8]==1 && m.values[0][9]==0);
    /* Transparent inverter feedback never settles; do not publish valid waves. */
    tm_init(&m);cell(&m,0,TM_LATCH,8,TM_ZERO,TM_ONE,0);gate(&m,0,TM_NOT,4,0);
    CHECK(!tm_run(&m));CHECK(!m.valid && m.error_column==0 && strstr(m.error,"32"));
    CHECK(!tm_explain(&m,0,&log) && log.count==33);
    CHECK(tm_logic((tm_gate_op)-1,0,0)==TM_X);CHECK(tm_logic(TM_AND,3,0)==TM_X);
    CHECK(!strcmp(tm_signal_name(13),"1") && !strcmp(tm_signal_name(14),"?"));
    CHECK(!strcmp(tm_type_name(TM_SR_LATCH),"SR latch") && !strcmp(tm_type_name((tm_type)-1),"?"));
    CHECK(!strcmp(tm_gate_name(TM_XOR),"XOR") && !strcmp(tm_gate_name((tm_gate_op)-1),"?"));
    CHECK(!strcmp(tm_reason_name(TM_SYNC_RESET),"Sync reset") && !strcmp(tm_reason_name((tm_reason)-1),"?"));
    return 0;
}
int tm_run_checks(unsigned *checks)
{
    if(!checks)return __LINE__;
    *checks=0;
    int line=local_devices(checks);
    if(!line)line=gates_unknown(checks);
    if(!line)line=clock_and_reset_boundaries(checks);
    if(!line)line=possible_reset_diagnostics(checks);
    if(!line)line=review_fixtures(checks);
    if(!line)line=boundaries(checks);
    return line;
}
