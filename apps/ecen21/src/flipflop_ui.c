#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdio.h>
#include "flipflop.h"
#include "flipflop_ui.h"
#include "info.h"
#include "utils.h"

#define ACCENT C_RGB(2, 12, 17)
static ff_state state;
static unsigned view, row;
static char const *const views[] = {"Simulation", "Characteristic", "Excitation", "History"};

static char const *event_name(ff_event event)
{
    return event == FF_RISING ? "RISE" : event == FF_FALLING ? "FALL" : "NONE";
}

static char bit(unsigned value)
{
    return value == FF_X ? 'X' : (char)('0' + value);
}

static bool two_inputs(void)
{
    return state.type == FF_SR || state.type == FF_JK;
}

static void simulation(void)
{
    if(state.q_valid) dprint(10, 48, ACCENT, "Current Q: %u | Steps: %u/16", state.q, state.count);
    else dtext(10, 48, C_RED, "Current Q: undefined. Reinitialize (F4).");
    if(two_inputs()) dprint(10, 71, C_BLACK, "%s: %u [1]        %s: %u [2]",
        ff_input_name(state.type, 0), state.a, ff_input_name(state.type, 1), state.b);
    else dprint(10, 71, C_BLACK, "%s: %u [1 toggles input]", ff_input_name(state.type, 0), state.a);
    dprint(10, 94, C_BLACK, "Clock event: %s (F5 changes event)", event_name(state.event));
    ff_transition next;
    if(!state.q_valid) dtext(10, 117, C_RED, "Next preview: unavailable until reset.");
    else {
        ff_evaluate(state.type, state.q, state.a, state.b, state.active_edge, state.event, &next);
        if(next.next_valid) dprint(10, 117, ACCENT, "Next preview: Q+=%u (%s)", next.next_q, ff_status_name(next.status));
        else dtext(10, 117, C_RED, "Next preview: INVALID (SR inputs 11)");
    }
    if(state.count) {
        ff_transition const *last = &state.history[state.count - 1];
        if(last->next_valid) dprint(10, 143, C_DARK, "Last step: Q %u -> %u | %s", last->q, last->next_q, ff_status_name(last->status));
        else dtext(10, 143, C_RED, "Last step: INVALID; no binary next Q.");
    }
    else dtext(10, 143, C_DARK, "No steps yet. Preview does not change Q.");
    if(state.error[0]) dtext(10, 169, C_RED, state.error);
    else dtext(10, 169, C_DARK, "Only the selected active edge samples input.");
    dtext(10, 192, C_DARK, "EXE/F6 step | OPTN help | EXIT home");
}

static void characteristic(void)
{
    unsigned count = ff_characteristic_count(state.type);
    row %= count;
    dprint(10, 46, C_DARK, "Q   %s   %s   Q+   Action (active edge)",
        ff_input_name(state.type, 0), two_inputs() ? ff_input_name(state.type, 1) : "-");
    for(unsigned i = 0; i < count; i++) {
        ff_transition t;
        ff_characteristic(state.type, i, &t);
        int y = 65 + (int)i * 15;
        if(i == row) drect(7, y - 1, 386, y + 12, C_RGB(26, 29, 31));
        dprint(10, y, t.next_valid ? C_BLACK : C_RED, "%u   %u   %c    %c   %s", t.q, t.a,
            two_inputs() ? bit(t.b) : '-', t.next_valid ? bit(t.next_q) : '!', ff_status_name(t.status));
    }
    dtext(10, 190, C_DARK, "UP/DOWN row | ! = undefined, never 0 or 1");
}

static void excitation(void)
{
    row %= 4;
    dprint(10, 48, C_DARK, "Current Q   Desired Q+     %s   %s",
        ff_input_name(state.type, 0), two_inputs() ? ff_input_name(state.type, 1) : "-");
    for(unsigned i = 0; i < 4; i++) {
        ff_excitation e;
        ff_get_excitation(state.type, i / 2, i % 2, &e);
        int y = 75 + (int)i * 23;
        if(i == row) drect(7, y - 2, 386, y + 14, C_RGB(26, 29, 31));
        dprint(10, y, C_BLACK, "    %u           %u          %c   %c", i / 2, i % 2,
            bit(e.a), two_inputs() ? bit(e.b) : '-');
    }
    dtext(10, 174, ACCENT, "X = either 0 or 1 is acceptable.");
    dtext(10, 192, C_DARK, "Not an unknown Q. Tables assume active edge.");
}

static void history(void)
{
    if(!state.count) {
        dtext(10, 70, C_DARK, "History is empty. F2 returns to simulation.");
        dtext(10, 100, C_DARK, "Each EXE/F6 step records one clock event.");
        return;
    }
    row %= state.count;
    unsigned first = row / 7 * 7;
    dprint(10, 46, C_DARK, "#   Q  %s %s Edge/Event Q+  Action", ff_input_name(state.type, 0),
        two_inputs() ? ff_input_name(state.type, 1) : "-");
    for(unsigned i = first; i < first + 7 && i < state.count; i++) {
        ff_transition const *t = &state.history[i];
        int y = 68 + (int)(i - first) * 17;
        if(i == row) drect(7, y - 2, 386, y + 13, C_RGB(26, 29, 31));
        dprint(10, y, t->next_valid ? C_BLACK : C_RED, "%2u  %u  %u %c  %c/%c       %c  %s", i + 1,
            t->q, t->a, two_inputs() ? bit(t->b) : '-',
            t->active_edge == FF_RISING ? 'R' : 'F',
            t->event == FF_RISING ? 'R' : t->event == FF_FALLING ? 'F' : '-',
            t->next_valid ? bit(t->next_q) : '!', ff_status_name(t->status));
    }
    dprint(10, 192, C_DARK, "R=rise F=fall -=none | Page %u/%u", first / 7 + 1, (state.count + 6) / 7);
}

static void draw(void)
{
    dclear(C_WHITE);
    info_title("ECEN21 - Flip-flops", ACCENT);
    dprint(10, 26, ACCENT, "%s | %s | Active: %s", ff_type_name(state.type), views[view], event_name(state.active_edge));
    if(view == 0) simulation();
    else if(view == 1) characteristic();
    else if(view == 2) excitation();
    else history();
    fkey_button(1, "TYPE"); fkey_button(2, "VIEW"); fkey_button(3, "EDGE");
    fkey_button(4, "RESET"); fkey_button(5, "EVENT"); fkey_button_color(6, "STEP", ACCENT);
    dupdate();
}

static void reset_prompt(void)
{
    dclear(C_WHITE); info_title("Initialize flip-flop simulation", ACCENT);
    dtext(10, 48, C_BLACK, "0: Start Q=0     1: Start Q=1");
    dtext(10, 78, C_DARK, "Clears history and any invalid-state lock.");
    dtext(10, 102, C_DARK, "Keeps inputs, clock event and active edge.");
    dtext(10, 130, C_DARK, "This is not a hardware reset/preset pin.");
    dtext(10, 174, ACCENT, "Other keys cancel without changes.");
    dupdate();
    int key = getkey().key;
    if(key == KEY_0 || key == KEY_1) {
        ff_reinitialize(&state, key == KEY_1 ? 1 : 0);
        row = 0; view = 0;
    }
}

static void help(void)
{
    dclear(C_WHITE); info_title("Flip-flop help", ACCENT);
    dtext(10, 28, C_BLACK, "1/2 toggle inputs (simulation view only).");
    dtext(10, 48, C_DARK, "F1 cycles SR -> D -> JK -> T -> SR.");
    dtext(10, 66, C_DARK, "Changing TYPE starts a new simulation.");
    dtext(10, 84, C_DARK, "F2: simulation / characteristic / excite / log");
    dtext(10, 102, C_DARK, "F3 active edge | F5 event: NONE/RISE/FALL");
    dtext(10, 120, C_DARK, "F4 initialize Q; clears all recorded steps.");
    dtext(10, 138, C_DARK, "SR is synchronous, active-high; not a latch.");
    dtext(10, 156, C_RED, "SR 11 on active edge: undefined; reset Q.");
    dtext(10, 174, C_DARK, "16 steps max. No electrical timing analysis.");
    dtext(10, 194, ACCENT, "Any key returns. EXIT in tool goes home.");
    dupdate(); getkey();
}

void flipflop_run(void)
{
    static bool initialized;
    if(!initialized) { ff_init(&state); initialized = true; }
    for(;;) {
        draw();
        int key = getkey().key;
        if(key == KEY_EXIT) return;
        if(key == INFO_KEY) { help(); continue; }
        if(key == KEY_F1) { ff_set_type(&state, (ff_type)((state.type + 1) % 4)); row = 0; view = 0; }
        else if(key == KEY_F2) { view = (view + 1) % 4; row = 0; }
        else if(key == KEY_F3) ff_set_edge(&state, state.active_edge == FF_RISING ? FF_FALLING : FF_RISING);
        else if(key == KEY_F4) reset_prompt();
        else if(key == KEY_F5) ff_set_event(&state, (ff_event)((state.event + 1) % 3));
        else if(view == 0 && (key == KEY_EXE || key == KEY_F6)) { ff_step(&state); row = state.count ? state.count - 1 : 0; }
        else if(view == 0 && key == KEY_1) ff_set_inputs(&state, state.a ^ 1, state.b);
        else if(view == 0 && key == KEY_2 && two_inputs()) ff_set_inputs(&state, state.a, state.b ^ 1);
        else if(key == KEY_UP || key == KEY_DOWN) {
            unsigned count = view == 1 ? ff_characteristic_count(state.type) : view == 2 ? 4 : view == 3 ? state.count : 0;
            if(count) row = key == KEY_UP ? (row + count - 1) % count : (row + 1) % count;
        }
    }
}
