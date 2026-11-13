#ifndef ECEN21_CONTROLLER_H
#define ECEN21_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>
#include "exact_boolean.h"

typedef enum { EC_READY, EC_SOLVED, EC_EMPTY, EC_ERROR } ec_status;
typedef struct {
    char input[5];
    bool replace_on_digit;
    ec_status status;
    eb_result result;
    char sop[160];
    unsigned verified_rows;
} ec_state;

void ec_load(ec_state *state, uint16_t mask);
void ec_clear(ec_state *state);
bool ec_digit(ec_state *state, unsigned digit);
void ec_backspace(ec_state *state);
bool ec_solve(ec_state *state);

#endif
