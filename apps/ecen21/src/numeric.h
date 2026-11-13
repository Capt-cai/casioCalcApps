#ifndef ECEN21_NUMERIC_H
#define ECEN21_NUMERIC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum { NM_ADD, NM_SUB } nm_operation;
typedef struct {
    unsigned width, base;
    bool signed_mode;
    nm_operation operation;
    char input[2][35];
    uint32_t operand[2], result;
    bool operand_valid[2], solved, carry, borrow, overflow, wrapped, truncated;
    char error[64];
} nm_state;

void nm_init(nm_state *state);
bool nm_set_input(nm_state *state, unsigned operand, const char *text);
bool nm_append(nm_state *state, unsigned operand, char character);
void nm_backspace(nm_state *state, unsigned operand);
void nm_clear(nm_state *state, unsigned operand);
bool nm_set_base(nm_state *state, unsigned base);
bool nm_set_width(nm_state *state, unsigned width);
bool nm_set_signed(nm_state *state, bool signed_mode);
void nm_set_operation(nm_state *state, nm_operation operation);
bool nm_solve(nm_state *state);
/* Invalid widths return zero; supported widths are1..32. */
uint32_t nm_mask(unsigned width);
int64_t nm_signed_value(uint32_t value, unsigned width);
/* Returns required length excluding NUL, or -1 for invalid arguments.
 * BIN/HEX are padded raw bits; signed_decimal affects DEC only.
 * A positive capacity always NUL-terminates valid output; NULL,0 queries size. */
int nm_format(uint32_t value, unsigned width, unsigned base,
    bool signed_decimal, char *buffer, size_t capacity);

#endif
