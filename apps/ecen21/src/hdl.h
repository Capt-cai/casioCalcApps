#ifndef ECEN21_HDL_H
#define ECEN21_HDL_H
#include <stdbool.h>
#include <stddef.h>
#define HG_CAPACITY 8192u
#define HG_LINE_WIDTH 46u
typedef enum { HG_BEHAVIORAL,HG_STRUCTURAL,HG_ENCODER,HG_D,HG_T,HG_JK,HG_SHIFT,HG_MODULO,HG_RING,HG_DATAFLOW,HG_KIND_COUNT } hg_kind;
typedef enum { HG_NO_RESET,HG_SYNC_RESET,HG_ASYNC_RESET } hg_reset;
typedef struct {
    hg_kind kind;
    unsigned width,inputs,modulus,seed,slot;
    bool falling,reset_high,reset_value,right;
    hg_reset reset;
} hg_config;
typedef struct {
    char code[HG_CAPACITY]; /* first member; struct has unsigned alignment */
    hg_config generated;
    unsigned length;
    bool valid;
    char error[80];
} hg_document;
void hg_init(hg_config *config,hg_kind kind);
bool hg_is_adder(hg_kind kind);
const char *hg_name(hg_kind kind);
const char *hg_module(hg_kind kind);
unsigned hg_min_width(unsigned modulus);
bool hg_generate(const hg_config *config,hg_document *document);
bool hg_matches(const hg_config *config,const hg_document *document);
/* Filename is bounded8.3 ASCII; source bytes are even-sized for BFile writes. */
bool hg_filename(const hg_config *config,char *out,size_t cap);
unsigned hg_pages_lines(const hg_document *document);
bool hg_line(const hg_document *document,unsigned visual_line,char *out,size_t cap);
#endif
