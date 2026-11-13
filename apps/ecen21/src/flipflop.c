#include "flipflop.h"
#include <string.h>

static bool valid_type(ff_type type)
{
    return type == FF_SR || type == FF_D || type == FF_JK || type == FF_T;
}
static bool valid_event(ff_event event)
{
    return event == FF_NONE || event == FF_RISING || event == FF_FALLING;
}
static bool valid_edge(ff_event edge)
{
    return edge == FF_RISING || edge == FF_FALLING;
}
static bool fail(ff_state *state, ff_status status, const char *message)
{
    if(state) {
        state->last_status = status;
        /* All messages are fixed literals shorter than the destination. */
        strcpy(state->error,message);
    }
    return false;
}
static void clear_error(ff_state *state)
{
    state->error[0] = '\0';
}

void ff_init(ff_state *state)
{
    if(!state) return;
    memset(state,0,sizeof *state);
    state->type = FF_D;
    state->active_edge = FF_RISING;
    state->event = FF_RISING;
    state->q_valid = true;
    state->last_status = FF_HOLD;
}

bool ff_reinitialize(ff_state *state, unsigned initial_q)
{
    if(!state || initial_q > 1)
        return fail(state,FF_BAD_ARGUMENT,"Initial Q must be 0 or 1");
    state->initial_q = initial_q;
    state->q = initial_q;
    state->q_valid = true;
    state->count = 0;
    memset(state->history,0,sizeof state->history);
    state->last_status = FF_HOLD;
    clear_error(state);
    return true;
}

bool ff_set_type(ff_state *state, ff_type type)
{
    if(!state || !valid_type(type))
        return fail(state,FF_BAD_ARGUMENT,"Invalid flip-flop type");
    if(state->type == type) return true;
    state->type = type;
    state->a = state->b = 0;
    state->event = FF_RISING;
    return ff_reinitialize(state,state->initial_q);
}

bool ff_set_inputs(ff_state *state, unsigned a, unsigned b)
{
    if(!state || a > 1 || b > 1)
        return fail(state,FF_BAD_ARGUMENT,"Inputs must be 0 or 1");
    state->a = a;
    state->b = b;
    clear_error(state);
    return true;
}

bool ff_set_edge(ff_state *state, ff_event active_edge)
{
    if(!state || !valid_edge(active_edge))
        return fail(state,FF_BAD_ARGUMENT,"Choose rising or falling edge");
    state->active_edge = active_edge;
    clear_error(state);
    return true;
}

bool ff_set_event(ff_state *state, ff_event event)
{
    if(!state || !valid_event(event))
        return fail(state,FF_BAD_ARGUMENT,"Invalid clock event");
    state->event = event;
    clear_error(state);
    return true;
}

ff_status ff_evaluate(ff_type type, unsigned q, unsigned a, unsigned b,
    ff_event active_edge, ff_event event, ff_transition *result)
{
    ff_transition next = {.q=q, .a=a, .b=b, .event=event,
        .active_edge=active_edge, .next_q=0, .next_valid=false,
        .status=FF_BAD_ARGUMENT};
    if(!result) return FF_BAD_ARGUMENT;
    if(!valid_type(type) || q > 1 || a > 1 || b > 1 ||
        !valid_edge(active_edge) || !valid_event(event)) {
        *result = next;
        return next.status;
    }
    next.next_q = q;
    next.next_valid = true;
    next.status = FF_HOLD;
    if(event == active_edge) {
        if(type == FF_D) {
            next.next_q = a;
            next.status = FF_CAPTURE;
        }
        else if(type == FF_T) {
            if(a) { next.next_q = 1-q; next.status = FF_TOGGLE; }
        }
        else if(a && b) {
            if(type == FF_SR) {
                next.next_q = 0; /* Deliberately unusable; see next_valid. */
                next.next_valid = false;
                next.status = FF_INVALID;
            }
            else { next.next_q = 1-q; next.status = FF_TOGGLE; }
        }
        else if(a) { next.next_q = 1; next.status = FF_SET; }
        else if(b) { next.next_q = 0; next.status = FF_RESET; }
    }
    *result = next;
    return next.status;
}

unsigned ff_characteristic_count(ff_type type)
{
    if(!valid_type(type)) return 0;
    return type == FF_D || type == FF_T ? 4 : 8;
}

bool ff_characteristic(ff_type type, unsigned index, ff_transition *result)
{
    unsigned count = ff_characteristic_count(type);
    if(!result || index >= count) return false;
    unsigned combinations = count / 2;
    unsigned inputs = index % combinations;
    unsigned a = combinations == 2 ? inputs : inputs / 2;
    unsigned b = combinations == 2 ? 0 : inputs % 2;
    ff_evaluate(type,index/combinations,a,b,FF_RISING,FF_RISING,result);
    return true;
}

bool ff_get_excitation(ff_type type, unsigned q, unsigned next_q,
    ff_excitation *result)
{
    if(!result || !valid_type(type) || q > 1 || next_q > 1) return false;
    ff_excitation answer = {0,0};
    if(type == FF_D) answer.a = next_q;
    else if(type == FF_T) answer.a = q != next_q;
    else if(type == FF_JK) {
        answer.a = q ? FF_X : next_q;
        answer.b = q ? 1-next_q : FF_X;
    }
    else {
        answer.a = q ? (next_q ? FF_X : 0) : next_q;
        answer.b = q ? 1-next_q : (next_q ? 0 : FF_X);
    }
    *result = answer;
    return true;
}

bool ff_step(ff_state *state)
{
    if(!state) return false;
    if(!state->q_valid)
        return fail(state,FF_LOCKED,"Q undefined: reinitialize before STEP");
    if(state->count >= FF_HISTORY_LIMIT)
        return fail(state,FF_HISTORY_FULL,"History full: reinitialize to clear");
    ff_transition next;
    ff_status status = ff_evaluate(state->type,state->q,state->a,state->b,
        state->active_edge,state->event,&next);
    if(status == FF_BAD_ARGUMENT)
        return fail(state,status,"Invalid simulation settings");
    state->history[state->count++] = next;
    state->last_status = status;
    if(!next.next_valid) {
        state->q_valid = false;
        return fail(state,FF_INVALID,"SR 11: Q undefined; reinitialize");
    }
    state->q = next.next_q;
    clear_error(state);
    return true;
}

const char *ff_type_name(ff_type type)
{
    switch(type) {
    case FF_SR: return "SR";
    case FF_D: return "D";
    case FF_JK: return "JK";
    case FF_T: return "T";
    default: return "?";
    }
}
const char *ff_input_name(ff_type type, unsigned index)
{
    if(!valid_type(type) || index > 1) return "?";
    if(type == FF_SR) return index ? "R" : "S";
    if(type == FF_JK) return index ? "K" : "J";
    if(index) return "-";
    return type == FF_D ? "D" : "T";
}
const char *ff_status_name(ff_status status)
{
    switch(status) {
    case FF_HOLD: return "Hold";
    case FF_CAPTURE: return "Capture D";
    case FF_SET: return "Set";
    case FF_RESET: return "Reset";
    case FF_TOGGLE: return "Toggle";
    case FF_INVALID: return "Invalid SR 11";
    case FF_BAD_ARGUMENT: return "Invalid argument";
    case FF_LOCKED: return "Reinitialize Q";
    case FF_HISTORY_FULL: return "History full";
    default: return "?";
    }
}
