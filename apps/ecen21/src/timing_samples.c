#include "timing_samples.h"
#include <string.h>

static tm_cell device(tm_type type,unsigned data,unsigned clock)
{
    tm_cell c={.enabled=true,.type=type,.a=data,.b=TM_ZERO,.clock=clock,
        .edge=FF_RISING,.high=true,.reset_mode=TM_RESET_NONE,.reset=TM_ZERO,
        .reset_high=true,.reset_value=0,.initial=0};
    return c;
}
static void event(tm_model *m,unsigned index,unsigned mask,unsigned values)
{
    tm_set_event(m,index,mask,values&mask);
}
static void clocks(tm_model *m,unsigned cycles)
{
    for(unsigned i=0;i<cycles*2;i++)event(m,i,1,i%2?0:1);
    tm_set_event_count(m,cycles*2);
}
static void gate(tm_model *m,unsigned index,tm_gate_op op,unsigned a,unsigned b)
{
    tm_gate g={.enabled=true,.op=op,.a=a,.b=b};tm_set_gate(m,index,&g);
}
const char *tm_sample_name(unsigned n)
{
    static const char *const names[]={"Maximum64: four T cells","Review1: master/slave","Review2: latch / rise / fall",
        "Review3: derived-clock ripple","Review4: synchronous4T","Review5: soft feedback reset",
        "Synchronous vs asynchronous reset","SR invalid / X propagation","SR level latch","Unstable feedback (error)"};
    return n<10?names[n]:"?";
}
bool tm_sample(tm_model *m,unsigned n)
{
    if(!m || n>9)return false;
    tm_init(m);
    for(unsigned i=0;i<TM_CELLS;i++)m->cells[i].enabled=false;
    for(unsigned i=0;i<TM_GATES;i++)m->gates[i].enabled=false;
    tm_set_event_count(m,0);tm_set_initial_inputs(m,0);
    if(n==1){
        tm_cell master=device(TM_LATCH,1,0),slave=device(TM_LATCH,4,0);slave.high=false;
        tm_set_cell(m,0,&master);tm_set_cell(m,1,&slave);gate(m,0,TM_NOT,5,TM_ZERO);
        static const unsigned masks[]={1,2,1,2,1,1,2,1,1};
        static const unsigned values[]={1,2,0,0,1,0,2,1,0};
        for(unsigned i=0;i<9;i++){event(m,i,masks[i],values[i]);}
        tm_set_event_count(m,9);
    }
    else if(n==2){
        tm_cell a=device(TM_LATCH,1,0),b=device(TM_D,1,0),c=device(TM_D,1,0);c.edge=FF_FALLING;
        tm_set_cell(m,0,&a);tm_set_cell(m,1,&b);tm_set_cell(m,2,&c);
        for(unsigned i=0;i<3;i++)gate(m,i,TM_NOT,4+i,TM_ZERO);
        static const unsigned masks[]={2,1,2,2,2,1,2,2,2,2,1,2,2,2,1,2};
        static const unsigned values[]={2,1,0,2,0,0,2,0,2,0,1,2,0,2,0,0};
        for(unsigned i=0;i<16;i++){event(m,i,masks[i],values[i]);}
        tm_set_event_count(m,16);
    }
    else if(n==3){
        for(unsigned i=0;i<3;i++){tm_cell c=device(TM_T,TM_ONE,i?3+i:0);tm_set_cell(m,i,&c);}clocks(m,8);
    }
    else if(n==4 || n==0){
        for(unsigned i=0;i<4;i++){tm_cell c=device(TM_T,i==0?TM_ONE:i==1?4:i==2?8:9,0);c.initial=n==4 && i<3?1:0;tm_set_cell(m,i,&c);}
        gate(m,0,TM_AND,4,5);gate(m,1,TM_AND,8,6);clocks(m,n==0?32:16);
    }
    else if(n==5){
        for(unsigned i=0;i<3;i++){tm_cell c=device(TM_T,i==0?TM_ONE:i==1?4:8,0);c.reset_mode=TM_RESET_SYNC;c.reset=9;c.reset_high=false;tm_set_cell(m,i,&c);}
        gate(m,0,TM_AND,4,5);gate(m,1,TM_NAND,4,6);clocks(m,8);
    }
    else if(n==6){
        tm_cell a=device(TM_D,TM_ONE,0),b=a;a.reset_mode=TM_RESET_SYNC;b.reset_mode=TM_RESET_ASYNC;a.reset=b.reset=2;
        tm_set_cell(m,0,&a);tm_set_cell(m,1,&b);
        static const unsigned masks[]={1,1,4,1,1,4,1};static const unsigned values[]={1,0,4,1,0,0,1};
        for(unsigned i=0;i<7;i++){event(m,i,masks[i],values[i]);}
        tm_set_event_count(m,7);
    }
    else if(n==7){
        tm_cell a=device(TM_SR,1,0),b=device(TM_D,4,0);a.b=3;tm_set_cell(m,0,&a);tm_set_cell(m,1,&b);
        static const unsigned masks[]={2,1,1,8,1,1,2,1,1,8,1};static const unsigned values[]={2,1,0,8,1,0,0,1,0,0,1};
        for(unsigned i=0;i<11;i++){event(m,i,masks[i],values[i]);}
        tm_set_event_count(m,11);
    }
    else if(n==8){
        tm_cell a=device(TM_SR_LATCH,1,0);a.b=3;tm_set_cell(m,0,&a);
        static const unsigned masks[]={2,1,8,2,1,8,2,1};static const unsigned values[]={2,1,8,0,0,0,2,1};
        for(unsigned i=0;i<8;i++){event(m,i,masks[i],values[i]);}
        tm_set_event_count(m,8);
    }
    else{
        tm_cell c=device(TM_LATCH,8,TM_ONE);tm_set_cell(m,0,&c);gate(m,0,TM_NOT,4,TM_ZERO);
    }
    tm_invalidate(m);return true;
}
