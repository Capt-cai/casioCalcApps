#include "flipflop.h"
#include "flipflop_checks.h"
#include <string.h>

#define CHECK(condition) do { ++*checks; if(!(condition)) return __LINE__; } while(0)

/* Independent truth tables, indexed Q*4 + a*2 + b. -1 means forbidden.
 * D/T intentionally ignore b, but both binary b values are tested. */
static int oracle(ff_type type, unsigned q, unsigned a, unsigned b)
{
    static const int sr[] = {0,0,1,-1, 1,0,1,-1};
    static const int d[] = {0,0,1,1, 0,0,1,1};
    static const int jk[] = {0,0,1,1, 1,0,1,0};
    static const int t[] = {0,0,1,1, 1,1,0,0};
    const int *table = type == FF_SR ? sr : type == FF_D ? d :
        type == FF_JK ? jk : t;
    return table[q*4+a*2+b];
}
static ff_status oracle_status(ff_type type, unsigned a, unsigned b)
{
    if(type == FF_D) return FF_CAPTURE;
    if(type == FF_T) return a ? FF_TOGGLE : FF_HOLD;
    static const ff_status statuses_sr[] = {FF_HOLD,FF_RESET,FF_SET,FF_INVALID};
    static const ff_status statuses_jk[] = {FF_HOLD,FF_RESET,FF_SET,FF_TOGGLE};
    return (type == FF_SR ? statuses_sr : statuses_jk)[a*2+b];
}
static bool matches(unsigned wanted, unsigned actual)
{
    return wanted == FF_X || wanted == actual;
}

static int transition_checks(unsigned *checks)
{
    for(unsigned type=FF_SR;type<=FF_T;type++)
    for(unsigned q=0;q<2;q++)
    for(unsigned a=0;a<2;a++)
    for(unsigned b=0;b<2;b++)
    for(unsigned edge=FF_RISING;edge<=FF_FALLING;edge++)
    for(unsigned event=FF_NONE;event<=FF_FALLING;event++) {
        ff_transition result;
        int expected = event == edge ? oracle((ff_type)type,q,a,b) : (int)q;
        ff_status expected_status = event == edge ?
            oracle_status((ff_type)type,a,b) : FF_HOLD;
        CHECK(ff_evaluate((ff_type)type,q,a,b,(ff_event)edge,
            (ff_event)event,&result) == expected_status);
        CHECK(result.next_valid == (expected >= 0));
        if(expected >= 0) CHECK(result.next_q == (unsigned)expected);
        CHECK(result.status == expected_status);
        CHECK(result.active_edge == (ff_event)edge);
        CHECK(result.q == q && result.a == a && result.b == b && result.event == (ff_event)event);
        ff_state state;
        ff_init(&state);
        CHECK(ff_set_type(&state,(ff_type)type));
        CHECK(ff_reinitialize(&state,q));
        CHECK(ff_set_inputs(&state,a,b));
        CHECK(ff_set_edge(&state,(ff_event)edge));
        CHECK(ff_set_event(&state,(ff_event)event));
        CHECK(ff_step(&state) == (expected >= 0));
        CHECK(state.count == 1 && state.last_status == expected_status);
        CHECK(state.q_valid == (expected >= 0));
        CHECK(state.history[0].next_valid == (expected >= 0));
        CHECK(state.history[0].event == (ff_event)event);
        CHECK(state.history[0].active_edge == (ff_event)edge);
        if(expected >= 0) CHECK(state.q == (unsigned)expected && !state.error[0]);
        else CHECK(state.q == q && state.error[0]);
    }
    return 0;
}
static int table_checks(unsigned *checks)
{
    for(unsigned type=FF_SR;type<=FF_T;type++) {
        bool dual = type == FF_SR || type == FF_JK;
        unsigned count = dual ? 8 : 4;
        CHECK(ff_characteristic_count((ff_type)type) == count);
        for(unsigned row=0;row<count;row++) {
            ff_transition result;
            unsigned q = row / (dual ? 4 : 2);
            unsigned a = dual ? (row%4)/2 : row%2;
            unsigned b = dual ? row%2 : 0;
            int expected = oracle((ff_type)type,q,a,b);
            CHECK(ff_characteristic((ff_type)type,row,&result));
            CHECK(result.q == q && result.a == a && result.b == b);
            CHECK(result.next_valid == (expected >= 0));
            CHECK(result.event == FF_RISING && result.active_edge == FF_RISING);
            CHECK(result.status == oracle_status((ff_type)type,a,b));
            if(expected >= 0) CHECK(result.next_q == (unsigned)expected);
        }
        for(unsigned q=0;q<2;q++)
        for(unsigned next=0;next<2;next++) {
            ff_excitation result;
            CHECK(ff_get_excitation((ff_type)type,q,next,&result));
            CHECK(result.a <= FF_X && result.b <= FF_X);
            /* Soundness AND completeness: every represented assignment works,
             * every binary assignment reaching next is represented. */
            for(unsigned a=0;a<2;a++)
            for(unsigned b=0;b<(dual ? 2u : 1u);b++) {
                bool included = matches(result.a,a) && matches(result.b,b);
                int actual = oracle((ff_type)type,q,a,b);
                CHECK(included == (actual == (int)next));
            }
        }
    }
    ff_excitation excitation;
    CHECK(ff_get_excitation(FF_JK,1,0,&excitation));
    CHECK(excitation.a == FF_X && excitation.b == 1);
    CHECK(ff_get_excitation(FF_SR,0,0,&excitation));
    CHECK(excitation.a == 0 && excitation.b == FF_X);
    CHECK(ff_get_excitation(FF_SR,1,1,&excitation));
    CHECK(excitation.a == FF_X && excitation.b == 0);
    return 0;
}
static int state_checks(unsigned *checks)
{
    ff_state state;
    ff_init(&state);
    CHECK(state.type == FF_D && state.q == 0 && state.initial_q == 0 && state.q_valid);
    CHECK(state.active_edge == FF_RISING && state.event == FF_RISING);
    CHECK(state.a == 0 && state.b == 0 && state.count == 0 && !state.error[0]);
    CHECK(ff_set_inputs(&state,1,0));
    CHECK(ff_set_event(&state,FF_NONE));
    CHECK(ff_step(&state) && state.q == 0);
    CHECK(ff_set_event(&state,FF_FALLING));
    CHECK(ff_step(&state) && state.q == 0);
    CHECK(ff_set_event(&state,FF_RISING));
    CHECK(ff_step(&state) && state.q == 1);
    CHECK(state.history[0].next_q == 0 && state.history[1].next_q == 0 && state.history[2].next_q == 1);
    CHECK(ff_set_type(&state,FF_JK));
    CHECK(state.q == 0 && state.count == 0 && state.a == 0 && state.b == 0);
    CHECK(ff_set_inputs(&state,1,1));
    CHECK(ff_step(&state) && state.q == 1);
    CHECK(ff_step(&state) && state.q == 0);
    CHECK(ff_set_type(&state,FF_T));
    CHECK(ff_set_inputs(&state,1,0));
    CHECK(ff_step(&state) && state.q == 1);
    CHECK(ff_set_inputs(&state,0,0));
    CHECK(ff_step(&state) && state.q == 1);
    CHECK(ff_set_inputs(&state,1,0));
    CHECK(ff_step(&state) && state.q == 0);
    CHECK(ff_set_type(&state,FF_SR));
    CHECK(ff_set_inputs(&state,1,0));
    CHECK(ff_step(&state) && state.q == 1);
    CHECK(ff_set_inputs(&state,1,1));
    CHECK(!ff_step(&state) && !state.q_valid && state.count == 2);
    CHECK(state.last_status == FF_INVALID && state.history[1].status == FF_INVALID);
    CHECK(!state.history[1].next_valid && state.q == 1 && state.error[0]);
    CHECK(!ff_step(&state) && state.count == 2 && state.last_status == FF_LOCKED);
    /* Settings/no-op type cannot bypass invalid lock. */
    CHECK(ff_set_type(&state,FF_SR));
    CHECK(!state.q_valid && state.count == 2);
    CHECK(ff_set_inputs(&state,0,0));
    CHECK(ff_set_edge(&state,FF_FALLING));
    CHECK(ff_set_event(&state,FF_NONE));
    CHECK(!ff_step(&state) && state.count == 2 && !state.q_valid);
    CHECK(ff_reinitialize(&state,1));
    CHECK(state.q_valid && state.q == 1 && state.initial_q == 1 && state.count == 0);
    CHECK(state.type == FF_SR && state.active_edge == FF_FALLING && state.event == FF_NONE);
    CHECK(state.a == 0 && state.b == 0 && !state.error[0]);
    CHECK(ff_step(&state) && state.q == 1);
    CHECK(ff_set_type(&state,FF_SR));
    CHECK(state.count == 1 && state.event == FF_NONE && state.q == 1);
    CHECK(ff_set_type(&state,FF_D));
    CHECK(state.count == 0 && state.initial_q == 1 && state.q == 1);
    CHECK(state.active_edge == FF_FALLING && state.event == FF_RISING && state.a == 0 && state.b == 0);
    CHECK(ff_set_event(&state,FF_FALLING));
    CHECK(ff_set_inputs(&state,1,0));
    for(unsigned i=0;i<FF_HISTORY_LIMIT;i++) {
        CHECK(ff_step(&state));
        CHECK(state.count == i+1 && state.q == 1 && state.q_valid);
    }
    ff_transition first = state.history[0], last = state.history[15];
    CHECK(!ff_step(&state));
    CHECK(state.count == 16 && state.last_status == FF_HISTORY_FULL && state.error[0]);
    CHECK(first.q == state.history[0].q && first.next_q == state.history[0].next_q);
    CHECK(first.a == state.history[0].a && first.b == state.history[0].b);
    CHECK(first.event == state.history[0].event && first.active_edge == state.history[0].active_edge);
    CHECK(first.status == state.history[0].status && first.next_valid == state.history[0].next_valid);
    CHECK(last.q == state.history[15].q && last.next_q == state.history[15].next_q);
    CHECK(last.a == state.history[15].a && last.b == state.history[15].b);
    CHECK(last.event == state.history[15].event && last.active_edge == state.history[15].active_edge);
    CHECK(last.status == state.history[15].status && last.next_valid == state.history[15].next_valid);
    CHECK(ff_set_inputs(&state,0,0));
    CHECK(ff_set_event(&state,FF_NONE));
    CHECK(ff_set_edge(&state,FF_RISING));
    CHECK(state.history[0].active_edge == FF_FALLING && state.history[15].active_edge == FF_FALLING);
    CHECK(ff_set_type(&state,FF_D));
    CHECK(!ff_step(&state) && state.count == 16);
    CHECK(ff_reinitialize(&state,0));
    CHECK(state.count == 0 && state.q_valid && state.q == 0 && !state.error[0]);
    CHECK(ff_step(&state) && state.count == 1);
    /* Explicitly changing type starts a fresh simulation after invalid input. */
    CHECK(ff_set_type(&state,FF_SR));
    CHECK(ff_set_inputs(&state,1,1));
    CHECK(ff_step(&state)==false && !state.q_valid);
    CHECK(ff_set_type(&state,FF_T));
    CHECK(state.q_valid && state.count == 0 && state.q == state.initial_q);
    return 0;
}
static int argument_checks(unsigned *checks)
{
    ff_transition out = {.q=42}, before = out;
    ff_excitation excitation = {42,43};
    CHECK(ff_evaluate(FF_D,0,0,0,FF_RISING,FF_RISING,NULL)==FF_BAD_ARGUMENT);
    CHECK(ff_evaluate((ff_type)-1,0,0,0,FF_RISING,FF_RISING,&out)==FF_BAD_ARGUMENT);
    CHECK(!out.next_valid);
    CHECK(ff_evaluate((ff_type)4,0,0,0,FF_RISING,FF_RISING,&out)==FF_BAD_ARGUMENT);
    CHECK(ff_evaluate(FF_D,2,0,0,FF_RISING,FF_RISING,&out)==FF_BAD_ARGUMENT);
    CHECK(ff_evaluate(FF_D,0,2,0,FF_RISING,FF_RISING,&out)==FF_BAD_ARGUMENT);
    CHECK(ff_evaluate(FF_D,0,0,2,FF_RISING,FF_RISING,&out)==FF_BAD_ARGUMENT);
    CHECK(ff_evaluate(FF_D,0,0,0,FF_NONE,FF_RISING,&out)==FF_BAD_ARGUMENT);
    CHECK(ff_evaluate(FF_D,0,0,0,(ff_event)-1,FF_RISING,&out)==FF_BAD_ARGUMENT);
    CHECK(ff_evaluate(FF_D,0,0,0,FF_RISING,(ff_event)3,&out)==FF_BAD_ARGUMENT);
    CHECK(ff_evaluate(FF_D,0,0,0,FF_RISING,(ff_event)-1,&out)==FF_BAD_ARGUMENT);
    out = before;
    CHECK(ff_characteristic_count((ff_type)-1)==0);
    CHECK(ff_characteristic_count((ff_type)4)==0);
    CHECK(!ff_characteristic(FF_D,4,&out));
    CHECK(!ff_characteristic(FF_SR,8,&out));
    CHECK(!ff_characteristic((ff_type)-1,0,&out));
    CHECK(!ff_characteristic(FF_D,0,NULL));
    CHECK(out.q==42);
    CHECK(!ff_get_excitation(FF_D,2,0,&excitation));
    CHECK(!ff_get_excitation(FF_D,0,2,&excitation));
    CHECK(!ff_get_excitation((ff_type)-1,0,0,&excitation));
    CHECK(!ff_get_excitation(FF_D,0,0,NULL));
    CHECK(excitation.a==42 && excitation.b==43);
    ff_init(NULL);
    CHECK(!ff_set_type(NULL,FF_D));
    CHECK(!ff_set_inputs(NULL,0,0));
    CHECK(!ff_set_edge(NULL,FF_RISING));
    CHECK(!ff_set_event(NULL,FF_NONE));
    CHECK(!ff_reinitialize(NULL,0));
    CHECK(!ff_step(NULL));
    ff_state state;
    ff_init(&state);
    CHECK(!ff_set_type(&state,(ff_type)-1) && state.type==FF_D);
    CHECK(!ff_set_inputs(&state,2,0) && state.a==0);
    CHECK(!ff_set_inputs(&state,0,2) && state.b==0);
    CHECK(!ff_set_edge(&state,FF_NONE) && state.active_edge==FF_RISING);
    CHECK(!ff_set_event(&state,(ff_event)3) && state.event==FF_RISING);
    CHECK(!ff_reinitialize(&state,2) && state.q==0 && state.count==0);
    state.type=(ff_type)99;
    CHECK(!ff_step(&state) && state.count==0 && state.last_status==FF_BAD_ARGUMENT);
    CHECK(strcmp(ff_type_name(FF_SR),"SR")==0);
    CHECK(strcmp(ff_type_name(FF_D),"D")==0);
    CHECK(strcmp(ff_type_name(FF_JK),"JK")==0);
    CHECK(strcmp(ff_type_name(FF_T),"T")==0);
    CHECK(strcmp(ff_type_name((ff_type)-1),"?")==0);
    CHECK(strcmp(ff_input_name(FF_SR,0),"S")==0);
    CHECK(strcmp(ff_input_name(FF_SR,1),"R")==0);
    CHECK(strcmp(ff_input_name(FF_JK,0),"J")==0);
    CHECK(strcmp(ff_input_name(FF_JK,1),"K")==0);
    CHECK(strcmp(ff_input_name(FF_D,0),"D")==0);
    CHECK(strcmp(ff_input_name(FF_T,0),"T")==0);
    CHECK(strcmp(ff_input_name(FF_T,1),"-")==0);
    CHECK(strcmp(ff_input_name(FF_T,2),"?")==0);
    CHECK(strcmp(ff_input_name((ff_type)-1,0),"?")==0);
    for(unsigned status=FF_HOLD;status<=FF_HISTORY_FULL;status++)
        CHECK(strcmp(ff_status_name((ff_status)status),"?")!=0);
    CHECK(strcmp(ff_status_name((ff_status)-1),"?")==0);
    return 0;
}
int ff_run_checks(unsigned *checks)
{
    if(!checks) return __LINE__;
    *checks = 0;
    int line = transition_checks(checks);
    if(!line) line = table_checks(checks);
    if(!line) line = state_checks(checks);
    if(!line) line = argument_checks(checks);
    return line;
}
