/* Recursive-descent grammar adapted from apps/logic/src/main.c.
 * Original project: Copyright (c) 2026 José Rodrigues, MIT License.
 * This bounded model adds diagnostics, editor/source state and exact SOP/POS;
 * it does not modify the upstream Logic implementation. */
#include "solver.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *text;
    unsigned position, row, variables, error_pos;
    const char *error;
} parser;

static bool whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static void skip(parser *p)
{
    while(whitespace(p->text[p->position])) p->position++;
}

static void fail(parser *p, const char *message)
{
    if(!p->error) { p->error = message; p->error_pos = p->position; }
}

static int parse_or(parser *p);

static int parse_atom(parser *p)
{
    skip(p);
    char c = p->text[p->position];
    if(c >= 'a' && c <= 'd') c = (char)(c - 'a' + 'A');
    if(c >= 'A' && c <= 'D') {
        unsigned variable = (unsigned)(c - 'A');
        if(variable >= p->variables) {
            fail(p, "Variable outside active count");
            return 0;
        }
        p->position++;
        return (int)((p->row >> (p->variables - 1 - variable)) & 1u);
    }
    if(c == '0' || c == '1') { p->position++; return c - '0'; }
    if(c == '(') {
        p->position++;
        int value = parse_or(p);
        skip(p);
        if(p->text[p->position] == ')') p->position++;
        else fail(p, "Expected closing parenthesis");
        return value;
    }
    fail(p, "Expected A-D, 0, 1 or (");
    return 0;
}

static int parse_not(parser *p)
{
    skip(p);
    char c = p->text[p->position];
    if(c == '!' || c == '~' || c == '/') {
        p->position++;
        return !parse_not(p);
    }
    int value = parse_atom(p);
    skip(p);
    while(!p->error && p->text[p->position] == '\'') {
        p->position++;
        value = !value;
        skip(p);
    }
    return value;
}

static bool factor_start(char c)
{
    return (c >= 'A' && c <= 'D') || (c >= 'a' && c <= 'd')
        || c == '0' || c == '1' || c == '(' || c == '!'
        || c == '~' || c == '/';
}

static int parse_and(parser *p)
{
    int value = parse_not(p);
    while(!p->error) {
        skip(p);
        char c = p->text[p->position];
        if(c == '*' || c == '.' || c == '&') p->position++;
        else if(!factor_start(c)) break;
        value &= parse_not(p);
    }
    return value;
}

static int parse_xor(parser *p)
{
    int value = parse_and(p);
    skip(p);
    while(!p->error && p->text[p->position] == '^') {
        p->position++;
        value ^= parse_and(p);
        skip(p);
    }
    return value;
}

static int parse_or(parser *p)
{
    int value = parse_xor(p);
    skip(p);
    while(!p->error && (p->text[p->position] == '+' || p->text[p->position] == '|')) {
        p->position++;
        value |= parse_xor(p);
        skip(p);
    }
    return value;
}

static uint16_t domain(unsigned variables)
{
    return (uint16_t)((1u << (1u << variables)) - 1u);
}

static void clear_solutions(bs_model *m)
{
    memset(m->result, 0, sizeof m->result);
    memset(m->formatted, 0, sizeof m->formatted);
    m->checked_rows = 0;
}

static void dirty(bs_model *m)
{
    clear_solutions(m);
    m->status = BS_DIRTY;
    m->error[0] = '\0';
    m->error_pos = 0;
}

static bool recompute(bs_model *m)
{
    dirty(m);
    m->source = BS_EXPRESSION;
    m->on = m->dc = 0;
    m->valid = false;
    parser p = {.text = m->expr, .variables = m->variables};
    skip(&p);
    if(!p.text[p.position]) {
        m->status = BS_EMPTY;
        m->error_pos = p.position;
        snprintf(m->error, sizeof m->error, "Enter an expression first");
        return false;
    }
    for(unsigned row = 0; row < (1u << m->variables); row++) {
        p = (parser){.text = m->expr, .variables = m->variables, .row = row};
        int value = parse_or(&p);
        skip(&p);
        if(p.text[p.position]) fail(&p, "Unexpected token");
        if(p.error) {
            m->on = 0;
            m->status = BS_SYNTAX;
            m->error_pos = p.error_pos;
            snprintf(m->error, sizeof m->error, "%s", p.error);
            return false;
        }
        if(value) m->on |= (uint16_t)(1u << row);
    }
    m->valid = true;
    return true;
}

void bs_init(bs_model *m)
{
    memset(m, 0, sizeof *m);
    m->variables = 4;
    bs_set_expression(m, "AB+C");
}

bool bs_set_expression(bs_model *m, const char *expression)
{
    if(!expression) return false;
    size_t n = 0;
    while(n < sizeof m->expr && expression[n]) n++;
    if(n == sizeof m->expr) return false;
    memmove(m->expr, expression, n + 1);
    m->cursor = (unsigned)n;
    return recompute(m);
}

bool bs_insert(bs_model *m, char character)
{
    size_t n = strlen(m->expr);
    if(character < ' ' || character > '~' || n >= sizeof m->expr - 1 || m->cursor > n)
        return false;
    memmove(m->expr + m->cursor + 1, m->expr + m->cursor, n - m->cursor + 1);
    m->expr[m->cursor++] = character;
    recompute(m);
    return true;
}

void bs_backspace(bs_model *m)
{
    size_t n = strlen(m->expr);
    if(!m->cursor || m->cursor > n) return;
    memmove(m->expr + m->cursor - 1, m->expr + m->cursor, n - m->cursor + 1);
    m->cursor--;
    recompute(m);
}

void bs_clear(bs_model *m)
{
    m->expr[0] = '\0';
    m->cursor = 0;
    recompute(m);
}

bool bs_set_variables(bs_model *m, unsigned variables)
{
    if(variables < 2 || variables > 4) return false;
    m->variables = variables;
    if(m->source == BS_EXPRESSION) recompute(m);
    else {
        dirty(m);
        m->on &= domain(variables);
        m->dc &= domain(variables);
        m->valid = true;
    }
    return true;
}

bool bs_cycle_row(bs_model *m, unsigned row)
{
    if(m->variables < 2 || m->variables > 4 || row >= (1u << m->variables)) return false;
    uint16_t bit = (uint16_t)(1u << row);
    if(m->dc & bit) m->dc &= (uint16_t)~bit;
    else if(m->on & bit) { m->on &= (uint16_t)~bit; m->dc |= bit; }
    else m->on |= bit;
    m->source = BS_TABLE;
    m->valid = true;
    dirty(m);
    return true;
}

bool bs_use_expression(bs_model *m)
{
    return recompute(m);
}

bool bs_solve(bs_model *m)
{
    clear_solutions(m);
    if(!m->valid) return false;
    if(m->variables < 2 || m->variables > 4 || (m->on & m->dc)
        || ((m->on | m->dc) & (uint16_t)~domain(m->variables))) {
        m->status = BS_ERROR;
        m->valid = false;
        snprintf(m->error, sizeof m->error, "Invalid truth-table data");
        return false;
    }
    for(int form = EB_SOP; form <= EB_POS; form++) {
        if(eb_minimize(m->variables, m->on, m->dc, (eb_form)form, &m->result[form]))
            goto failed;
        int n = eb_format(&m->result[form], m->formatted[form], sizeof m->formatted[form]);
        if(n < 0 || (size_t)n >= sizeof m->formatted[form]) goto failed;
        for(unsigned row = 0; row < (1u << m->variables); row++) {
            int value = eb_evaluate(&m->result[form], row);
            if(value < 0 || value > 1) goto failed;
            if(!(m->dc & (1u << row)) && value != (int)((m->on >> row) & 1u)) goto failed;
        }
    }
    for(unsigned row = 0; row < (1u << m->variables); row++)
        if(!(m->dc & (1u << row))) m->checked_rows++;
    m->status = BS_SOLVED;
    m->error[0] = '\0';
    m->error_pos = 0;
    return true;

failed:
    clear_solutions(m);
    m->status = BS_ERROR;
    snprintf(m->error, sizeof m->error, "Exact cover verification failed");
    return false;
}

unsigned bs_kmap_row(unsigned variables, unsigned grid_row, unsigned grid_col)
{
    static const unsigned gray[4] = {0, 1, 3, 2};
    if(variables < 2 || variables > 4) return UINT_MAX;
    unsigned rows = variables == 4 ? 4 : 2;
    unsigned cols = variables == 2 ? 2 : 4;
    if(grid_row >= rows || grid_col >= cols) return UINT_MAX;
    return (gray[grid_row] << (variables == 2 ? 1 : 2)) | gray[grid_col];
}

int bs_row_value(const bs_model *m, unsigned row)
{
    if(!m->valid || m->variables < 2 || m->variables > 4 || row >= (1u << m->variables)) return -1;
    if(m->dc & (1u << row)) return 2;
    return (int)((m->on >> row) & 1u);
}
