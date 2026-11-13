#ifndef ECEN21_SEQ_CONNECTIONS_H
#define ECEN21_SEQ_CONNECTIONS_H
#include "sequential.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CN_MAX_NODES 224u
#define CN_ABSENT UINT16_MAX
#define CN_LINE_CAPACITY 47u

typedef enum { CN_ZERO, CN_ONE, CN_INPUT, CN_NOT, CN_AND, CN_OR, CN_XOR } cn_op;
typedef struct {
    cn_op op;
    unsigned count;
    uint16_t input[4];
} cn_node;
typedef struct {
    bool valid, reverse;
    eb_form form;
    unsigned count, variables, pin_count, bits, initial, recovery;
    sq_policy policy;
    ff_event edge;
    ff_type types[3];
    char variable_names[4][3], pin_names[6][4];
    unsigned pin_bits[6];
    cn_node nodes[CN_MAX_NODES];
    uint16_t pin_drives[6], output_drives[2];
} cn_netlist;

/* Compile all required pin functions and specified outputs into an acyclic
 * shared declarative gate netlist. No heap/recursion. Failure invalidates out. */
bool cn_build(const sq_model *model, eb_form form, cn_netlist *out);
/* Standalone snapshot line presentation: every line fits46 visible chars.
 * Short capacity or bad index returns false and NUL-terminates when possible. */
unsigned cn_line_count(const cn_netlist *net);
bool cn_line(const cn_netlist *net, unsigned index, char *text, size_t capacity);
#endif
