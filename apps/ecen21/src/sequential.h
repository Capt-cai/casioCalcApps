#ifndef ECEN21_SEQUENTIAL_H
#define ECEN21_SEQUENTIAL_H
#include "flipflop.h"
#include "exact_boolean.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SQ_UNDEFINED 255u
#define SQ_MAX_ROWS 16u
#define SQ_MAX_PINS 6u
#define SQ_MAX_TRACE 32u
/* Text fields0/1: cycles;2..7: bit*2+input equations;8/9: Z0/Z1;10:E timeline. */
typedef enum { SQ_RETURN, SQ_HOLD, SQ_DC } sq_policy;
typedef struct {
    char name[4];
    unsigned bit, input;
    uint16_t on, dc;
    eb_result solution[2];
    unsigned parity_mask[2];
    bool parity_invert[2];
} sq_pin;
typedef struct {
    unsigned q, e, target, next[2];
    bool used, care;
    unsigned excitation[SQ_MAX_PINS];
} sq_row;
typedef struct {
    /* Specification: types[0] is Q0; equations[bit*2] is a (D/T/J/S),
     * equations[bit*2+1] is b (K/R, unused for D/T). */
    unsigned bits;
    bool controlled, reverse;
    ff_type types[3];
    ff_event active_edge;
    sq_policy policy;
    unsigned recovery, initial, fixed_e;
    char cycle_text[2][32], equations[6][80], outputs[2][80], timeline[33];
    /* Derived data. Treat as unavailable whenever valid is false. */
    unsigned variables, count, pin_count, checked_rows;
    sq_pin pins[SQ_MAX_PINS];
    sq_row rows[SQ_MAX_ROWS];
    uint16_t output_mask[2];
    bool output_present[2], valid;
    char error[80];
    unsigned error_field, error_pos;
    uint16_t invalid_text_fields; /* Oversize edit remains invalid until repaired. */
} sq_model;
typedef struct {
    unsigned states[SQ_MAX_TRACE+1], controls[SQ_MAX_TRACE];
    unsigned steps, prefix, period;
    bool invalid, limited, cycle;
} sq_trace_result;

void sq_init(sq_model *model);
void sq_invalidate(sq_model *model);
bool sq_set_bits(sq_model *model, unsigned bits);
bool sq_set_controlled(sq_model *model, bool controlled);
bool sq_set_reverse(sq_model *model, bool reverse);
bool sq_set_type(sq_model *model, unsigned bit, ff_type type);
bool sq_set_edge(sq_model *model, ff_event edge);
bool sq_set_policy(sq_model *model, sq_policy policy);
bool sq_set_initial(sq_model *model, unsigned initial);
bool sq_set_recovery(sq_model *model, unsigned recovery);
bool sq_set_fixed_e(sq_model *model, unsigned e);
const char *sq_text(const sq_model *model, unsigned field);
bool sq_set_text(sq_model *model, unsigned field, const char *text);
bool sq_solve(sq_model *model);
/* Portable table entry for host/integration fixtures. Each target row uses
 * (Q<<controlled)|E ordering; used/DC masks use this same row index. */
bool sq_synthesize_table(sq_model *model, const unsigned targets[16],
    uint16_t used_mask, uint16_t dc_mask);
int sq_pin_value(const sq_model *model, eb_form form, unsigned pin, unsigned row);
int sq_format_equation(const sq_model *model, eb_form form, unsigned pin,
    char *buffer, size_t capacity);
/* Output value is present-state/pre-update. -1 means absent/invalid query. */
int sq_output_value(const sq_model *model, unsigned output, unsigned row);
bool sq_next(const sq_model *model, eb_form form, unsigned q, unsigned e,
    ff_event event, unsigned *next);
/* timeline empty/NULL: fixed-E traversal until cycle/invalid. Otherwise a
 * maximum32-character0/1 timeline, no cycle inference from repeatedQ. */
bool sq_trace(const sq_model *model, eb_form form, unsigned start, unsigned e,
    const char *timeline, sq_trace_result *result);
/* Returns Q-label or E for the A-D variable mapping, or '?' on invalid. */
const char *sq_variable_name(const sq_model *model, unsigned variable);
const char *sq_policy_name(sq_policy policy);
#endif
