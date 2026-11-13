#ifndef ECEN21_SOLVER_H
#define ECEN21_SOLVER_H

#include <stdbool.h>
#include <stdint.h>
#include "exact_boolean.h"

typedef enum { BS_EXPRESSION, BS_TABLE } bs_source;
typedef enum { BS_DIRTY, BS_SOLVED, BS_SYNTAX, BS_EMPTY, BS_ERROR } bs_status;

typedef struct {
    char expr[80];
    unsigned cursor, variables;
    uint16_t on, dc;
    bs_source source;
    bs_status status;
    bool valid;
    unsigned error_pos; /* zero-based source offset; UI may display +1 */
    char error[64];
    eb_result result[2];
    char formatted[2][256];
    unsigned checked_rows;
} bs_model;

void bs_init(bs_model *model);
/* Stores a bounded expression and recomputes truth data; false on syntax or
 * empty input. NULL/oversize input is rejected without changing the model. */
bool bs_set_expression(bs_model *model, const char *expression);
/* Editor methods switch to expression mode only after an actual edit.
 * Insertion returns whether a printable character was inserted. */
bool bs_insert(bs_model *model, char character);
void bs_backspace(bs_model *model);
void bs_clear(bs_model *model);
/* Returns false for counts outside 2..4, without changing the model.
 * Table mode keeps only low-domain rows; added rows are zero. Expression
 * mode reevaluates, possibly making the saved expression invalid. */
bool bs_set_variables(bs_model *model, unsigned variables);
/* Explicit table edit: 0 -> 1 -> X -> 0. After an invalid expression this
 * starts from its cleared truth data; it never silently reapplies expr. */
bool bs_cycle_row(bs_model *model, unsigned row);
bool bs_solve(bs_model *model);
/* Gray-code layout: 2x2, 2x4 or 4x4. UINT_MAX marks invalid coordinates. */
unsigned bs_kmap_row(unsigned variables, unsigned grid_row, unsigned grid_col);
/* 0, 1, 2 (X), or -1 when the model/row is invalid. */
int bs_row_value(const bs_model *model, unsigned row);
/* Explicitly replace edited truth data by the saved expression's value. */
bool bs_use_expression(bs_model *model);

#endif
