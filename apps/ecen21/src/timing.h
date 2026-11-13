#ifndef ECEN21_TIMING_H
#define ECEN21_TIMING_H
#include "flipflop.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TM_INPUTS 4u
#define TM_CELLS 4u
#define TM_GATES 4u
#define TM_SIGNALS 12u
#define TM_EVENTS 64u
#define TM_PHASES 32u
#define TM_ZERO 12u
#define TM_ONE 13u
#define TM_X 2u

typedef enum { TM_SR, TM_D, TM_JK, TM_T, TM_LATCH, TM_SR_LATCH } tm_type;
typedef enum { TM_NOT, TM_AND, TM_OR, TM_XOR, TM_NAND } tm_gate_op;
typedef enum { TM_RESET_NONE, TM_RESET_SYNC, TM_RESET_ASYNC } tm_reset_mode;
typedef enum {
    TM_INITIAL, TM_HOLD, TM_CAPTURE, TM_TOGGLE, TM_SET, TM_CLEAR,
    TM_TRANSPARENT, TM_SYNC_RESET, TM_ASYNC_RESET, TM_UNKNOWN,
    TM_CLOCK_UNCERTAIN
} tm_reason;
typedef struct {
    bool enabled;
    tm_type type;
    unsigned a, b, clock;
    ff_event edge;       /* D/T/JK/SR active edge. */
    bool high;           /* Latch transparent-high versus transparent-low. */
    tm_reset_mode reset_mode;
    unsigned reset;
    bool reset_high;
    unsigned reset_value, initial; /* Reset0/1; initial0/1/X. */
} tm_cell;
typedef struct { bool enabled; tm_gate_op op; unsigned a,b; } tm_gate;
typedef struct { uint8_t mask, values; } tm_event;
typedef struct {
    unsigned initial_inputs, event_count;
    tm_cell cells[TM_CELLS];
    tm_gate gates[TM_GATES];
    tm_event events[TM_EVENTS];
    /* Column0 is settled initialization; column i+1 follows event i. */
    uint8_t values[TM_EVENTS+1][TM_SIGNALS];
    uint8_t reasons[TM_EVENTS+1][TM_CELLS];
    uint8_t phases[TM_EVENTS+1];
    bool valid;
    unsigned completed, error_column, error_cell;
    char error[96];
} tm_model;

typedef struct {
    unsigned column, count;
    /* Point0 is post-external-input/pre-state-commit; subsequent points are
     * simultaneous state commits followed by combinational evaluation. */
    uint8_t values[TM_PHASES+1][TM_SIGNALS];
    uint8_t reasons[TM_PHASES+1][TM_CELLS];
} tm_phase_log;

void tm_init(tm_model *model);
void tm_invalidate(tm_model *model);
bool tm_set_cell(tm_model *model,unsigned cell,const tm_cell *value);
bool tm_set_gate(tm_model *model,unsigned gate,const tm_gate *value);
bool tm_set_initial_inputs(tm_model *model,unsigned value);
bool tm_set_event(tm_model *model,unsigned index,unsigned mask,unsigned values);
bool tm_set_event_count(tm_model *model,unsigned count);
bool tm_run(tm_model *model);
/* Bounded replay through selected column without modifying the source model.
 * False means validation/nonconvergence failure, never a usable stable trace. */
bool tm_explain(const tm_model *model,unsigned column,tm_phase_log *log);
/* Helpers usable by native UI and independent host fixtures. */
unsigned tm_logic(tm_gate_op op,unsigned a,unsigned b);
const char *tm_signal_name(unsigned signal);
const char *tm_type_name(tm_type type);
const char *tm_gate_name(tm_gate_op op);
const char *tm_reason_name(tm_reason reason);
#endif
