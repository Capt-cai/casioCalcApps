#include "controller.h"
#include <stdio.h>
#include <string.h>

static void invalidate(ec_state *state, ec_status status)
{
    state->status = status;
    state->sop[0] = '\0';
    state->verified_rows = 0;
    memset(&state->result, 0, sizeof state->result);
}

void ec_load(ec_state *state, uint16_t mask)
{
    snprintf(state->input, sizeof state->input, "%04X", (unsigned)mask);
    state->replace_on_digit = true;
    invalidate(state, EC_READY);
}

void ec_clear(ec_state *state)
{
    state->input[0] = '\0';
    state->replace_on_digit = false;
    invalidate(state, EC_READY);
}

bool ec_digit(ec_state *state, unsigned digit)
{
    static char const hex[] = "0123456789ABCDEF";
    if(digit > 15) return false;
    size_t n = state->replace_on_digit ? 0 : strlen(state->input);
    if(n == sizeof state->input - 1) return false;
    state->input[n] = hex[digit];
    state->input[n + 1] = '\0';
    state->replace_on_digit = false;
    invalidate(state, EC_READY);
    return true;
}

void ec_backspace(ec_state *state)
{
    size_t n = strlen(state->input);
    if(n) state->input[n - 1] = '\0';
    state->replace_on_digit = false;
    invalidate(state, EC_READY);
}

bool ec_solve(ec_state *state)
{
    static char const hex[] = "0123456789ABCDEF";
    invalidate(state, EC_ERROR);
    if(!state->input[0]) { state->status = EC_EMPTY; return false; }
    unsigned mask = 0;
    for(size_t i = 0; state->input[i]; i++) {
        char const *p = strchr(hex, state->input[i]);
        if(!p || i >= 4) return false;
        mask = (mask << 4) | (unsigned)(p - hex);
    }
    if(eb_minimize(4, mask, 0, EB_SOP, &state->result) != 0) return false;
    int n = eb_format(&state->result, state->sop, sizeof state->sop);
    if(n < 0 || (size_t)n >= sizeof state->sop) {
        invalidate(state, EC_ERROR);
        return false;
    }
    /* Check the selected cover against the actual input on the target too. */
    for(unsigned row = 0; row < 16; row++) {
        if(eb_evaluate(&state->result, row) != (int)((mask >> row) & 1)) {
            invalidate(state, EC_ERROR);
            return false;
        }
        state->verified_rows++;
    }
    state->status = EC_SOLVED;
    state->replace_on_digit = true;
    return true;
}
