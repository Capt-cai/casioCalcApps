#ifndef ECEN21_EXACT_BOOLEAN_H
#define ECEN21_EXACT_BOOLEAN_H
#include <stddef.h>
#include <stdint.h>

/* Portable experimental core: rows use A as the most significant bit.
 * ON and DC must be disjoint and confined to 2^variables rows; 2 <= n <= 4.
 * Optimal cost: fewest cubes, then fewest literals, then canonical cube order.
 * POS cubes cover OFF rows; each output clause is the complement of a cube.
 */
typedef enum { EB_SOP = 0, EB_POS = 1 } eb_form;
typedef struct { uint8_t bits, fixed; } eb_cube;
typedef struct {
    uint8_t variables, count, literals;
    eb_form form;
    eb_cube cubes[16];
} eb_result;
/* Returns 0 on success, -1 on invalid input (output is untouched). */
int eb_minimize(unsigned variables, uint32_t on, uint32_t dc,
                eb_form form, eb_result *output);
/* Evaluates the represented function; returns -1 for invalid row/result. */
int eb_evaluate(const eb_result *result, unsigned row);
/* Returns required length excluding NUL, or -1 for invalid arguments.
 * Always NUL-terminates when capacity > 0, including truncated output. */
int eb_format(const eb_result *result, char *buffer, size_t capacity);
#endif
