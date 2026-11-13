#ifndef ECEN21_COMBINATIONAL_H
#define ECEN21_COMBINATIONAL_H
#include "solver.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define CB_NODES 512u
#define CB_NONE UINT16_MAX
/* Inputs are numbered in displayed MSB-first order; no global IR is introduced. */
typedef enum { CB_HA, CB_FA, CB_POPCOUNT, CB_FUNCTION, CB_ENCODER, CB_DISPLAY, CB_RIPPLE } cb_kind;
typedef enum { CB_ZERO, CB_ONE, CB_INPUT, CB_NOT, CB_AND, CB_OR, CB_XOR } cb_op;
typedef struct { cb_op op; uint16_t a,b; } cb_node;
typedef struct {
    cb_kind kind;
    unsigned variables,output,encoder_inputs,width,a,b,cin;
    bool active_low;
    unsigned invalid_policy; /* 0=zero physical pins,1=blank,2=DC */
    uint16_t on,dc;
    char expression[80];
    bool expression_source;
    unsigned select_count,select[3];
    eb_form form;
    bool valid;
    char error[80];
    uint16_t truth,cares;
    eb_result result[2],cofactor[8];
    cb_node nodes[CB_NODES];
    unsigned node_count;
    uint16_t gate_output,mux_data[8],valid_output;
    unsigned verified;
} cb_model;
typedef struct { unsigned x,y,cin,sum,cout; } cb_stage;
void cb_init(cb_model *m,cb_kind kind);
void cb_dirty(cb_model *m);
unsigned cb_outputs(const cb_model *m);
unsigned cb_inputs(const cb_model *m);
const char *cb_name(cb_kind kind);
void cb_input_name(const cb_model *m,unsigned input,char *text,size_t cap);
void cb_output_name(const cb_model *m,char *text,size_t cap);
/* Solve only selected output; family selections share structured requirements. */
bool cb_solve(cb_model *m);
/* Expected truth from problem specification: -1 DC, -2 invalid row/model. */
int cb_expected(const cb_model *m,unsigned row);
int cb_eval_gate(const cb_model *m,unsigned row,bool validity);
int cb_eval_mux(const cb_model *m,unsigned row);
unsigned cb_encoder_code(unsigned pattern,unsigned inputs,bool *valid);
bool cb_ripple(unsigned width,unsigned a,unsigned b,unsigned cin,cb_stage stages[8],unsigned *sum,unsigned *cout);
bool cb_connection_line(const cb_model *m,unsigned line,char *text,size_t cap);
unsigned cb_connection_lines(const cb_model *m);
#endif
