#include "controller.h"
#include "controller_checks.h"
#include <string.h>

#define CHECK(condition) do { ++*checks; if(!(condition)) return __LINE__; } while(0)

int ec_run_checks(unsigned *checks)
{
    *checks = 0;
    ec_state state = {0};
    struct { uint16_t mask; unsigned terms, literals; char const *sop; } cases[] = {
        {0x007e, 3, 9, "A'C'D+A'BD'+A'B'C"},
        {0, 0, 0, "0"}, {0xffff, 1, 0, "1"},
        {0xaaaa, 1, 1, "D"}, {0xcccc, 1, 1, "C"},
        {0xf0f0, 1, 1, "B"}, {0xff00, 1, 1, "A"},
        {0xfccc, 2, 3, "C+AB"}, {0x6996, 8, 32, NULL},
    };
    for(unsigned i = 0; i < sizeof cases / sizeof *cases; i++) {
        ec_load(&state, cases[i].mask);
        CHECK(state.status == EC_READY && state.sop[0] == '\0');
        CHECK(ec_solve(&state));
        CHECK(state.status == EC_SOLVED && state.verified_rows == 16);
        CHECK(state.result.count == cases[i].terms);
        CHECK(state.result.literals == cases[i].literals);
        if(cases[i].sop) CHECK(strcmp(state.sop, cases[i].sop) == 0);
        for(unsigned row = 0; row < 16; row++)
            CHECK(eb_evaluate(&state.result, row) == ((cases[i].mask >> row) & 1));
    }
    CHECK(ec_digit(&state, 10));
    CHECK(strcmp(state.input, "A") == 0 && state.status == EC_READY);
    CHECK(state.sop[0] == '\0' && state.verified_rows == 0);
    CHECK(ec_digit(&state, 11) && ec_digit(&state, 12) && ec_digit(&state, 13));
    CHECK(strcmp(state.input, "ABCD") == 0);
    CHECK(!ec_digit(&state, 14) && strcmp(state.input, "ABCD") == 0);
    CHECK(!ec_digit(&state, 16) && !ec_digit(&state, 100));
    ec_backspace(&state);
    CHECK(strcmp(state.input, "ABC") == 0);
    CHECK(ec_solve(&state));
    ec_clear(&state);
    CHECK(state.status == EC_READY && !state.input[0] && !state.sop[0]);
    CHECK(!ec_solve(&state) && state.status == EC_EMPTY);
    CHECK(!state.verified_rows);
    ec_backspace(&state);
    CHECK(state.status == EC_READY && !state.input[0]);
    CHECK(ec_digit(&state, 0) && ec_digit(&state, 0) &&
          ec_digit(&state, 7) && ec_digit(&state, 14));
    CHECK(ec_solve(&state) && state.result.count == 3 && state.result.literals == 9);
    ec_backspace(&state);
    CHECK(state.status == EC_READY && !state.sop[0] && !state.verified_rows);
    CHECK(strcmp(state.input, "007") == 0);
    CHECK(ec_digit(&state, 14) && ec_solve(&state));
    CHECK(strcmp(state.sop, "A'C'D+A'BD'+A'B'C") == 0);
    ec_load(&state, 0x1234);
    CHECK(ec_digit(&state, 1) && strcmp(state.input, "1") == 0);
    state.input[0] = 'G';
    CHECK(!ec_solve(&state) && state.status == EC_ERROR && !state.sop[0]);
    return 0;
}
