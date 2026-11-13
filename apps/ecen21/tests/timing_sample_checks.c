#include "timing_samples.h"
#include "timing_sample_checks.h"
#include <string.h>
#define CHECK(x) do{++*checks;if(!(x))return __LINE__;}while(0)
static unsigned state(const tm_model *m,unsigned column,unsigned bits)
{
    unsigned v=0;for(unsigned b=0;b<bits;b++)v|=(unsigned)m->values[column][4+b]<<b;return v;
}
int ts_run_checks(unsigned *checks)
{
    if(!checks)return __LINE__;
    *checks=0;tm_model m;tm_phase_log log;
    CHECK(tm_sample(&m,1));CHECK(tm_run(&m));CHECK(m.event_count==9 && m.completed==10);
    static const unsigned qm[]={0,0,1,1,1,0,0,0,1,1};
    static const unsigned qs[]={0,0,0,1,1,1,0,0,0,1};
    for(unsigned col=0;col<10;col++){CHECK(m.values[col][4]==qm[col]);CHECK(m.values[col][5]==qs[col]);CHECK(m.values[col][8]==1-qs[col]);}
    CHECK(tm_set_event(&m,2,3,0));CHECK(!m.valid);CHECK(tm_run(&m));
    CHECK(m.values[3][4]==1 && m.values[3][5]==1); /* Closing master doesn't capture newD. */
    CHECK(tm_sample(&m,2));CHECK(tm_run(&m));CHECK(m.event_count==16);
    static const unsigned qa[]={0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1};
    static const unsigned qb[]={0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0};
    for(unsigned col=0;col<17;col++){
        CHECK(m.values[col][4]==qa[col]);CHECK(m.values[col][5]==qb[col]);CHECK(m.values[col][6]==(col>=15?1u:0u));
        for(unsigned i=0;i<3;i++)CHECK(m.values[col][8+i]==1-m.values[col][4+i]);
    }
    CHECK(tm_sample(&m,3));CHECK(tm_run(&m));
    for(unsigned i=0;i<8;i++){CHECK(state(&m,2*i+1,3)==7-i);CHECK(state(&m,2*i+2,3)==7-i);}
    CHECK(tm_explain(&m,1,&log));CHECK(log.count>=4);
    CHECK(log.values[0][4]==0 && log.values[0][5]==0 && log.values[0][6]==0);
    CHECK(log.values[1][4]==1 && log.values[1][5]==0 && log.values[1][6]==0);
    CHECK(log.values[2][4]==1 && log.values[2][5]==1 && log.values[2][6]==0);
    CHECK(log.values[log.count-1][4]==1 && log.values[log.count-1][5]==1 && log.values[log.count-1][6]==1);
    CHECK(tm_sample(&m,4));CHECK(tm_run(&m));CHECK(state(&m,0,4)==7);
    for(unsigned i=0;i<16;i++){
        unsigned expected=(8+i)%16;CHECK(state(&m,2*i+1,4)==expected);CHECK(state(&m,2*i+2,4)==expected);
        CHECK(m.values[2*i+1][8]==((expected&3u)==3u));CHECK(m.values[2*i+1][9]==((expected&7u)==7u));
    }
    CHECK(tm_sample(&m,5));CHECK(tm_run(&m));
    static const unsigned soft[]={1,2,3,4,5,0,1,2};
    for(unsigned i=0;i<8;i++){CHECK(state(&m,2*i+1,3)==soft[i]);CHECK(state(&m,2*i+2,3)==soft[i]);}
    CHECK(m.values[9][9]==0);CHECK(m.values[11][9]==1);
    CHECK(m.reasons[11][0]==TM_SYNC_RESET && m.reasons[11][1]==TM_SYNC_RESET && m.reasons[11][2]==TM_SYNC_RESET);
    CHECK(tm_explain(&m,11,&log));CHECK(log.values[0][9]==0);CHECK(log.values[log.count-1][9]==1);
    CHECK(tm_sample(&m,6));CHECK(tm_run(&m));
    static const unsigned sync[]={0,1,1,1,0,0,0,1},async[]={0,1,1,0,0,0,0,1};
    for(unsigned i=0;i<8;i++){CHECK(m.values[i][4]==sync[i]);CHECK(m.values[i][5]==async[i]);}
    CHECK(m.reasons[3][1]==TM_ASYNC_RESET && m.reasons[4][0]==TM_SYNC_RESET);
    CHECK(tm_sample(&m,7));CHECK(tm_run(&m));
    CHECK(m.values[2][4]==1 && m.values[2][5]==0);
    CHECK(m.values[5][4]==TM_X && m.values[5][5]==1);
    CHECK(m.values[8][4]==0 && m.values[8][5]==TM_X);
    CHECK(m.values[11][4]==0 && m.values[11][5]==0);
    CHECK(tm_sample(&m,8));CHECK(tm_run(&m));
    static const unsigned sr[]={0,0,1,TM_X,0,0,0,0,1};
    for(unsigned i=0;i<9;i++)CHECK(m.values[i][4]==sr[i]);
    CHECK(tm_sample(&m,0));CHECK(tm_run(&m));CHECK(m.event_count==64 && m.completed==65);
    for(unsigned i=0;i<32;i++){CHECK(state(&m,2*i+1,4)==(i+1)%16);CHECK(state(&m,2*i+2,4)==(i+1)%16);}
    CHECK(!tm_set_event(&m,64,1,1));CHECK(!m.valid);CHECK(tm_sample(&m,0));CHECK(tm_run(&m));
    CHECK(tm_set_event_count(&m,0));CHECK(tm_run(&m));CHECK(m.completed==1);
    CHECK(tm_sample(&m,9));CHECK(!tm_run(&m));CHECK(!m.valid && m.error[0]);CHECK(!tm_explain(&m,0,&log));
    CHECK(tm_sample(&m,1));CHECK(tm_run(&m));
    CHECK(tm_sample(&m,0));
    tm_cell jk=m.cells[0];jk.type=TM_JK;jk.b=TM_ONE;
    CHECK(tm_set_cell(&m,0,&jk));CHECK(tm_run(&m));
    CHECK(m.values[1][4]==1 && m.values[3][4]==0);
    CHECK(m.reasons[1][0]==TM_TOGGLE && m.reasons[3][0]==TM_TOGGLE);
    CHECK(!tm_sample(NULL,1));CHECK(!tm_sample(&m,10));CHECK(!strcmp(tm_sample_name(10),"?"));
    return 0;
}
