#include "solver.h"
#include "solver_checks.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define CHECK(condition) do { ++*checks; if(!(condition)) return __LINE__; } while(0)

static void table(bs_model *m, unsigned variables, unsigned on, unsigned dc)
{
    bs_init(m);
    bs_set_expression(m, "0");
    bs_set_variables(m, variables);
    for(unsigned row = 0; row < (1u << variables); row++) {
        if((on | dc) & (1u << row)) bs_cycle_row(m, row);
        if(dc & (1u << row)) bs_cycle_row(m, row);
    }
}

static unsigned count_bits(unsigned value)
{
    unsigned count = 0;
    while(value) { count += value & 1u; value >>= 1; }
    return count;
}

int bs_run_checks(unsigned *checks)
{
    *checks = 0;
    bs_model m;
    bs_init(&m);
    CHECK(m.variables == 4 && m.on == 0xfccc && !m.dc);
    CHECK(m.valid && m.status == BS_DIRTY && m.source == BS_EXPRESSION);
    CHECK(m.cursor == 4 && strcmp(m.expr, "AB+C") == 0);
    CHECK(bs_solve(&m) && m.status == BS_SOLVED && m.checked_rows == 16);
    CHECK(strcmp(m.formatted[EB_SOP], "C+AB") == 0);
    CHECK(m.result[EB_SOP].count == 2 && m.result[EB_SOP].literals == 3);
    CHECK(strcmp(m.formatted[EB_POS], "(B+C)*(A+C)") == 0);
    CHECK(m.result[EB_POS].count == 2 && m.result[EB_POS].literals == 4);

    struct { const char *text; unsigned on; } expressions[] = {
        {"A",0xff00}, {"B",0xf0f0}, {"C",0xcccc}, {"D",0xaaaa},
        {"a + b",0xfff0}, {"A^B",0x0ff0}, {"A+B*C",0xffc0},
        {"A^B+C",0xcffc}, {"A|B&C",0xffc0}, {"A.B",0xf000},
        {"(A+B)(A+C)",0xffc0}, {"!A",0x00ff}, {"~A",0x00ff},
        {"/A",0x00ff}, {"A'",0x00ff}, {"A ''",0xff00},
        {"!!A",0xff00}, {"(A+B)'",0x000f}, {"A'B",0x00f0},
        {"1",0xffff}, {"0",0}, {"\tA\n+\rB ",0xfff0},
        {"A+!A",0xffff}, {"A*!A",0}, {"A^A",0},
        {"A'(B^C+C^D)",0x007e}, {"A^B^C^D",0x6996},
    };
    for(unsigned i = 0; i < sizeof expressions / sizeof *expressions; i++) {
        CHECK(bs_set_expression(&m, expressions[i].text));
        CHECK(m.valid && m.on == expressions[i].on && !m.dc);
        CHECK(m.status == BS_DIRTY && !m.formatted[0][0] && !m.formatted[1][0]);
        CHECK(!m.result[0].count && !m.result[1].count && !m.checked_rows);
        CHECK(bs_solve(&m));
        for(unsigned row = 0; row < 16; row++) {
            int expected = (int)((expressions[i].on >> row) & 1u);
            CHECK(bs_row_value(&m, row) == expected);
            CHECK(eb_evaluate(&m.result[EB_SOP], row) == expected);
            CHECK(eb_evaluate(&m.result[EB_POS], row) == expected);
        }
    }

    /* Independent direct Boolean semantics, not a second recursive parser.
     * Every choice of a,b,c,d in !(a+b)c^d is checked on all 16 rows. */
    for(unsigned a = 0; a < 4; a++) for(unsigned b = 0; b < 4; b++)
    for(unsigned c = 0; c < 4; c++) for(unsigned d = 0; d < 4; d++) {
        char text[24];
        snprintf(text, sizeof text, "!(%c+%c)%c^%c", 'A'+(int)a, 'A'+(int)b,
            'A'+(int)c, 'A'+(int)d);
        CHECK(bs_set_expression(&m, text));
        unsigned expected = 0;
        for(unsigned row = 0; row < 16; row++) {
            unsigned av = (row >> (3-a)) & 1u, bv = (row >> (3-b)) & 1u;
            unsigned cv = (row >> (3-c)) & 1u, dv = (row >> (3-d)) & 1u;
            unsigned value = ((!(av | bv)) & cv) ^ dv;
            expected |= value << row;
            CHECK(bs_row_value(&m, row) == (int)value);
        }
        CHECK(m.on == expected);
        if(((a*64+b*16+c*4+d) % 17) == 0) CHECK(bs_solve(&m));
    }

    struct { const char *text; unsigned position; } invalid[] = {
        {"A+",2}, {"(A",2}, {"A)",1}, {"E",0}, {"A?B",1},
        {"A++B",2}, {"A**B",2}, {"()",1}, {"'A",0},
        {"!",1}, {"A^^B",2}, {"1. ",3},
    };
    for(unsigned i = 0; i < sizeof invalid / sizeof *invalid; i++) {
        CHECK(!bs_set_expression(&m, invalid[i].text));
        CHECK(!m.valid && m.status == BS_SYNTAX && m.error[0]);
        CHECK(m.error_pos == invalid[i].position);
        CHECK(!m.on && !m.dc && !m.checked_rows);
        CHECK(!m.formatted[0][0] && !m.formatted[1][0]);
        CHECK(!bs_solve(&m) && m.status == BS_SYNTAX);
        CHECK(bs_row_value(&m, 0) == -1);
    }
    CHECK(!bs_set_expression(&m, " \t\r\n"));
    CHECK(m.status == BS_EMPTY && !m.valid && !m.on && !m.dc);
    CHECK(bs_set_expression(&m, "A"));
    CHECK(bs_set_variables(&m, 2) && m.on == 0xc && m.valid);
    CHECK(!bs_set_expression(&m, "C+A"));
    CHECK(m.status == BS_SYNTAX && m.error_pos == 0);
    CHECK(bs_set_variables(&m, 3) && m.valid && m.on == 0xfa);
    CHECK(bs_set_variables(&m, 4) && m.valid && m.on == 0xffcc);

    /* Every table edit cycles 0/1/X and invalidates both solved forms. */
    table(&m, 4, 0, 0);
    CHECK(bs_solve(&m));
    CHECK(bs_cycle_row(&m, 5) && bs_row_value(&m, 5) == 1);
    CHECK(m.on == 0x20 && !m.dc && m.source == BS_TABLE);
    CHECK(m.status == BS_DIRTY && !m.checked_rows && !m.result[0].count);
    CHECK(!m.formatted[0][0] && !m.formatted[1][0]);
    CHECK(bs_solve(&m) && m.checked_rows == 16);
    CHECK(bs_cycle_row(&m, 5) && bs_row_value(&m, 5) == 2);
    CHECK(!m.on && m.dc == 0x20 && !m.checked_rows);
    CHECK(bs_solve(&m) && m.checked_rows == 15);
    CHECK(bs_cycle_row(&m, 5) && bs_row_value(&m, 5) == 0);
    CHECK(!m.on && !m.dc);
    CHECK(bs_solve(&m));
    CHECK(strcmp(m.formatted[EB_SOP], "0") == 0);
    CHECK(strcmp(m.formatted[EB_POS], "0") == 0);

    table(&m, 4, 0x007e, 0);
    CHECK(bs_solve(&m));
    CHECK(strcmp(m.formatted[EB_SOP], "A'C'D+A'BD'+A'B'C") == 0);
    CHECK(m.result[EB_SOP].count == 3 && m.result[EB_SOP].literals == 9);
    CHECK(m.result[EB_POS].count == 3 && m.result[EB_POS].literals == 7);
    for(unsigned form = 0; form < 2; form++) for(unsigned row = 0; row < 16; row++)
        CHECK(eb_evaluate(&m.result[form], row) == (int)((0x007e >> row) & 1u));
    table(&m, 2, 2, 1);
    CHECK(bs_solve(&m) && m.checked_rows == 3);
    CHECK(strcmp(m.formatted[EB_SOP], "A'") == 0);
    CHECK(strcmp(m.formatted[EB_POS], "(A')") == 0);
    CHECK(m.result[0].count == 1 && m.result[0].literals == 1);
    CHECK(m.result[1].count == 1 && m.result[1].literals == 1);
    CHECK(bs_set_expression(&m, "A'B") && m.on == 2);
    CHECK(bs_cycle_row(&m, 0) && bs_cycle_row(&m, 0));
    CHECK(m.on == 2 && m.dc == 1 && bs_row_value(&m, 0) == 2);
    CHECK(bs_solve(&m) && strcmp(m.formatted[EB_SOP], "A'") == 0);
    CHECK(m.result[EB_SOP].count == 1 && m.result[EB_SOP].literals == 1);
    CHECK(m.checked_rows == 3);
    for(unsigned variables = 2; variables <= 4; variables++) {
        unsigned mask = (1u << (1u << variables)) - 1u;
        table(&m, variables, 0, mask);
        CHECK(bs_solve(&m) && m.checked_rows == 0);
        CHECK(strcmp(m.formatted[EB_SOP], "0") == 0);
        CHECK(strcmp(m.formatted[EB_POS], "1") == 0);
        CHECK(!m.result[0].count && !m.result[1].count);
        table(&m, variables, mask, 0);
        CHECK(bs_solve(&m));
        CHECK(strcmp(m.formatted[0], "1") == 0 && strcmp(m.formatted[1], "1") == 0);
    }

    /* Saved text is retained but cannot silently replace table edits. */
    bs_init(&m);
    CHECK(bs_cycle_row(&m, 0));
    CHECK(strcmp(m.expr, "AB+C") == 0 && m.on == 0xfccd);
    CHECK(bs_solve(&m) && m.on == 0xfccd && m.source == BS_TABLE);
    CHECK(bs_use_expression(&m) && m.on == 0xfccc && m.source == BS_EXPRESSION);
    CHECK(!m.checked_rows && !m.formatted[0][0]);
    CHECK(bs_cycle_row(&m, 0));
    m.cursor = 0;
    bs_backspace(&m);
    CHECK(m.source == BS_TABLE && m.on == 0xfccd);
    CHECK(bs_insert(&m, '!'));
    CHECK(m.source == BS_EXPRESSION && strcmp(m.expr, "!AB+C") == 0);
    CHECK(m.cursor == 1 && !m.dc && m.on == 0xccfc);
    bs_backspace(&m);
    CHECK(strcmp(m.expr, "AB+C") == 0 && m.cursor == 0 && m.on == 0xfccc);
    bs_clear(&m);
    CHECK(m.status == BS_EMPTY && !m.valid && !m.expr[0] && !m.cursor);
    CHECK(bs_cycle_row(&m, 2));
    CHECK(m.valid && m.source == BS_TABLE && m.on == 4 && !m.error[0]);
    CHECK(!bs_use_expression(&m) && m.status == BS_EMPTY && !m.on);

    table(&m, 4, 0x8001, 0x4002);
    CHECK(bs_set_variables(&m, 2));
    CHECK(m.source == BS_TABLE && m.on == 1 && m.dc == 2);
    CHECK(bs_set_variables(&m, 4) && m.on == 1 && m.dc == 2);
    CHECK(bs_row_value(&m, 15) == 0);
    bs_model before = m;
    CHECK(!bs_cycle_row(&m, 16) && memcmp(&m, &before, sizeof m) == 0);
    CHECK(!bs_set_variables(&m, 1) && memcmp(&m, &before, sizeof m) == 0);
    CHECK(!bs_set_variables(&m, 5) && memcmp(&m, &before, sizeof m) == 0);
    CHECK(bs_row_value(&m, 16) == -1);

    char longest[80];
    memset(longest, '!', 78); longest[78] = 'A'; longest[79] = '\0';
    CHECK(bs_set_expression(&m, longest) && m.on == 0xff00 && m.cursor == 79);
    before = m;
    CHECK(!bs_insert(&m, '+') && memcmp(&m, &before, sizeof m) == 0);
    char oversized[81]; memset(oversized, 'A', 80); oversized[80] = '\0';
    CHECK(!bs_set_expression(&m, oversized) && memcmp(&m, &before, sizeof m) == 0);
    CHECK(!bs_set_expression(&m, NULL) && memcmp(&m, &before, sizeof m) == 0);
    CHECK(!bs_insert(&m, '\n') && memcmp(&m, &before, sizeof m) == 0);
    memset(longest, '(', 39); longest[39] = 'A';
    memset(longest+40, ')', 39); longest[79] = '\0';
    CHECK(bs_set_expression(&m, longest) && m.on == 0xff00);
    CHECK(bs_set_expression(&m, "AB"));
    m.cursor = 1;
    CHECK(bs_insert(&m, '+') && strcmp(m.expr, "A+B") == 0 && m.cursor == 2);
    CHECK(m.on == 0xfff0);
    bs_backspace(&m);
    CHECK(strcmp(m.expr, "AB") == 0 && m.on == 0xf000);
    m.cursor = 80;
    before = m;
    CHECK(!bs_insert(&m, 'C') && memcmp(&m, &before, sizeof m) == 0);
    bs_backspace(&m);
    CHECK(memcmp(&m, &before, sizeof m) == 0);

    /* Partial editor input can nest deeper than a balanced valid string.
     * Exercise all 79 opening parentheses/prefix operators on target too. */
    memset(longest, '(', 79); longest[79] = '\0';
    CHECK(!bs_set_expression(&m, longest));
    CHECK(m.status == BS_SYNTAX && m.error_pos == 79);
    CHECK(!m.on && !m.dc && !m.formatted[0][0] && !m.formatted[1][0]);
    CHECK(!bs_solve(&m) && m.status == BS_SYNTAX);
    memset(longest, '!', 79); longest[79] = '\0';
    CHECK(!bs_set_expression(&m, longest));
    CHECK(m.status == BS_SYNTAX && m.error_pos == 79);

    /* K-map layout agrees with binary row labels, is bijective, and all
     * horizontal/vertical wraparound neighbours differ in exactly one bit. */
    for(unsigned variables = 2; variables <= 4; variables++) {
        unsigned rows = variables == 4 ? 4 : 2, cols = variables == 2 ? 2 : 4;
        unsigned visited = 0;
        for(unsigned r = 0; r < rows; r++) for(unsigned c = 0; c < cols; c++) {
            unsigned row = bs_kmap_row(variables, r, c);
            unsigned expected = ((r ^ (r >> 1)) << (variables == 2 ? 1 : 2)) | (c ^ (c >> 1));
            CHECK(row == expected && row < (1u << variables));
            CHECK(!(visited & (1u << row)));
            visited |= 1u << row;
            CHECK(count_bits(row ^ bs_kmap_row(variables, r, (c+1)%cols)) == 1);
            CHECK(count_bits(row ^ bs_kmap_row(variables, (r+1)%rows, c)) == 1);
        }
        CHECK(visited == (1u << (1u << variables)) - 1u);
        CHECK(bs_kmap_row(variables, rows, 0) == UINT_MAX);
        CHECK(bs_kmap_row(variables, 0, cols) == UINT_MAX);
        /* Every possible cube covers precisely 2^(n-fixed) cells. */
        for(unsigned fixed = 0; fixed < (1u << variables); fixed++)
            for(unsigned bits = 0; bits < (1u << variables); bits++) {
                if(bits & ~fixed) continue;
                unsigned cells = 0;
                for(unsigned r = 0; r < rows; r++) for(unsigned c = 0; c < cols; c++)
                    if((bs_kmap_row(variables, r, c) & fixed) == bits) cells++;
                CHECK(cells == (1u << (variables-count_bits(fixed))));
            }
    }
    CHECK(bs_kmap_row(1, 0, 0) == UINT_MAX && bs_kmap_row(5, 0, 0) == UINT_MAX);
    bs_init(&m); m.on = m.dc = 1;
    CHECK(!bs_solve(&m) && m.status == BS_ERROR && !m.valid);
    CHECK(!m.formatted[0][0] && !m.formatted[1][0]);
    return 0;
}
