#ifndef ECEN21_FLIPFLOP_H
#define ECEN21_FLIPFLOP_H

#include <stdbool.h>

#define FF_HISTORY_LIMIT 16u
#define FF_X 2u

typedef enum { FF_SR, FF_D, FF_JK, FF_T } ff_type;
typedef enum { FF_NONE, FF_RISING, FF_FALLING } ff_event;
typedef enum {
    FF_HOLD, FF_CAPTURE, FF_SET, FF_RESET, FF_TOGGLE, FF_INVALID,
    FF_BAD_ARGUMENT, FF_LOCKED, FF_HISTORY_FULL
} ff_status;
typedef struct {
    unsigned q, a, b;
    ff_event event, active_edge;
    unsigned next_q; /* Only usable when next_valid is true. */
    bool next_valid;
    ff_status status;
} ff_transition;
typedef struct { unsigned a, b; } ff_excitation;
typedef struct {
    ff_type type;
    ff_event active_edge, event;
    unsigned initial_q, q, a, b, count;
    bool q_valid;
    ff_status last_status;
    ff_transition history[FF_HISTORY_LIMIT];
    char error[64];
} ff_state;

/* Default: D, Q=0, D=0, rising active edge and rising event. */
void ff_init(ff_state *state);
/* Changing type explicitly resets to initial Q, inputs0 and rising event;
 * preserves active edge. Choosing the same type preserves everything. */
bool ff_set_type(ff_state *state, ff_type type);
bool ff_set_inputs(ff_state *state, unsigned a, unsigned b);
bool ff_set_edge(ff_state *state, ff_event active_edge);
bool ff_set_event(ff_state *state, ff_event event);
/* Sets initial/current Q and clears history/invalid lock; keeps settings/input. */
bool ff_reinitialize(ff_state *state, unsigned initial_q);
ff_status ff_evaluate(ff_type type, unsigned q, unsigned a, unsigned b,
    ff_event active_edge, ff_event event, ff_transition *result);
unsigned ff_characteristic_count(ff_type type);
/* Rows are Q-major, then a, then b; D/T have b=0. Uses active rising event. */
bool ff_characteristic(ff_type type, unsigned index, ff_transition *result);
bool ff_get_excitation(ff_type type, unsigned q, unsigned next_q,
    ff_excitation *result);
/* Accepted invalid SR step is recorded but returns false, invalidates Q and
 * locks the simulation until reinitialization/type change. */
bool ff_step(ff_state *state);
const char *ff_type_name(ff_type type);
const char *ff_input_name(ff_type type, unsigned index);
const char *ff_status_name(ff_status status);

#endif
