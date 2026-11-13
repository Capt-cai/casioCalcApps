#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdio.h>
#include <string.h>
#include "numeric.h"
#include "numeric_ui.h"
#include "info.h"
#include "keys.h"
#include "utils.h"

#define ACCENT C_RGB(2, 12, 17)
#define WARNING C_RGB(27, 10, 1)

static nm_state state;
static unsigned selected;
static bool result_view;

static char const *base_name(void)
{
    return state.base == 10 ? "DEC" : state.base == 16 ? "HEX" : "BIN";
}

static void binary_text(uint32_t value, char out[40])
{
    char raw[35];
    nm_format(value, state.width, 2, false, raw, sizeof raw);
    unsigned n = 0;
    for(unsigned i = 0; raw[i]; i++) {
        if(i && !((state.width - i) % 4)) out[n++] = ' ';
        out[n++] = raw[i];
    }
    out[n] = '\0';
}

static void status_line(void)
{
    dprint(10, 26, C_DARK, "%u-bit | %s | %s | A %s B", state.width,
        state.signed_mode ? "SIGNED" : "UNSIGNED", base_name(),
        state.operation == NM_ADD ? "+" : "-");
}

static void message(char const *text, int y, int color)
{
    size_t length = strlen(text), n = length > 46 ? 46 : length;
    char line[47];
    memcpy(line, text, n); line[n] = '\0'; dtext(10, y, color, line);
    if(length > n) dtext(10, y + 12, color, text + n);
}

static void conversions(uint32_t value, int y)
{
    char text[40];
    nm_format(value, state.width, 10, state.signed_mode, text, sizeof text);
    dprint(10, y, C_BLACK, "DEC: %s", text);
    nm_format(value, state.width, 16, false, text, sizeof text);
    dprint(10, y + 17, C_BLACK, "HEX: %s", text);
    binary_text(value, text);
    dprint(10, y + 34, C_BLACK, "BIN: %s", text);
}

static void draw_input(void)
{
    for(unsigned i = 0; i < 2; i++) {
        int y = 48 + (int)i * 28;
        dprint(10, y + 6, selected == i ? ACCENT : C_DARK, "%c", 'A' + (int)i);
        drect_border(30, y, 384, y + 23, C_WHITE, selected == i ? 2 : 1,
            selected == i ? ACCENT : C_LIGHT);
        dtext(39, y + 6, state.operand_valid[i] ? C_BLACK : C_RED,
            state.input[i][0] ? state.input[i] : "_");
    }
    if(state.operand_valid[selected]) {
        dprint(10, 111, ACCENT, "%c conversions:", 'A' + (int)selected);
        conversions(state.operand[selected], 128);
    }
    else {
        dprint(10, 112, C_RED, "%c needs valid input.", 'A' + (int)selected);
        dtext(10, 143, C_DARK, "Digits append; AC clears selected operand.");
        dtext(10, 160, C_DARK, "(-) enters a minus sign in signed DEC.");
    }
    if(state.error[0]) message(state.error, 181, C_RED);
    else if(state.truncated) message("Width reduction changed an operand value.", 186, WARNING);
    else dtext(10, 189, C_DARK, "UP/DOWN select | DEL / AC | EXE calculate");
}

static void draw_result(void)
{
    if(!state.solved) {
        dtext(10, 70, ACCENT, "No current arithmetic result.");
        dtext(10, 94, C_DARK, "Press EXE to calculate, or F5 to edit.");
        if(state.error[0]) message(state.error, 130, C_RED);
        return;
    }
    char a[35], b[35];
    nm_format(state.operand[0], state.width, 10, state.signed_mode, a, sizeof a);
    nm_format(state.operand[1], state.width, 10, state.signed_mode, b, sizeof b);
    dprint(10, 46, C_DARK, "A: %s", a);
    dprint(204, 46, C_DARK, "B: %s", b);
    conversions(state.result, 69);
    dline(10, 124, 384, 124, C_LIGHT);
    if(state.operation == NM_ADD)
        dprint(10, 135, ACCENT, "Carry: %u | Borrow: N/A", (unsigned)state.carry);
    else dprint(10, 135, ACCENT, "Carry: N/A | Borrow: %u", (unsigned)state.borrow);
    dprint(10, 153, state.overflow ? WARNING : ACCENT,
        "Signed overflow: %u", (unsigned)state.overflow);
    dtext(10, 172, state.wrapped ? WARNING : ACCENT, state.wrapped ?
        (state.signed_mode ? "Wrapped result: signed range exceeded." : "Wrapped result: unsigned range exceeded.") :
        (state.signed_mode ? "Result fits signed range." : "Result fits unsigned range."));
    if(state.truncated) dtext(10, 190, WARNING, "Width reduction changed an operand value.");
    else dtext(10, 190, C_DARK, "F5 edit | OPTN samples | EXIT home");
}

static void draw(void)
{
    dclear(C_WHITE);
    info_title("ECEN21 - Binary tools", ACCENT);
    status_line();
    if(result_view) draw_result(); else draw_input();
    fkey_button(1, "BASE"); fkey_button(2, "+/-");
    fkey_button(3, "SIGN"); fkey_button(4, "WIDTH");
    fkey_button(5, result_view ? "EDIT" : "RESULT");
    fkey_button_color(6, "CALC", ACCENT);
    dupdate();
}

static void samples(void)
{
    dclear(C_WHITE); info_title("Numeric samples", ACCENT);
    dtext(10, 29, C_DARK, "Choose 1-8 (replaces numeric inputs):");
    dtext(10, 51, C_BLACK, "1  8-bit unsigned: 255 + 1");
    dtext(10, 69, C_BLACK, "2  8-bit signed: 127 + 1");
    dtext(10, 87, C_BLACK, "3  8-bit unsigned: 0 - 1");
    dtext(10, 105, C_BLACK, "4  8-bit signed: -128 - 1");
    dtext(10, 123, C_BLACK, "5  16-bit signed: 32767 + 1");
    dtext(10, 141, C_BLACK, "6  32-bit unsigned: max + 1");
    dtext(10, 159, C_BLACK, "7  32-bit signed: max + 1");
    dtext(10, 177, C_BLACK, "8  8-bit signed conversion: -5");
    dtext(10, 198, C_DARK, "Other keys return without changes.");
    dupdate();
    int sample = key_digit(getkey().key);
    if(sample < 1 || sample > 8) return;
    nm_init(&state);
    if(sample == 5) nm_set_width(&state, 16);
    if(sample == 6 || sample == 7) nm_set_width(&state, 32);
    if(sample == 2 || sample == 4 || sample == 5 || sample == 7 || sample == 8)
        nm_set_signed(&state, true);
    if(sample == 2) nm_set_input(&state, 0, "127");
    if(sample == 3) nm_set_input(&state, 0, "0");
    if(sample == 4) nm_set_input(&state, 0, "-128");
    if(sample == 5) nm_set_input(&state, 0, "32767");
    if(sample == 6) nm_set_input(&state, 0, "4294967295");
    if(sample == 7) nm_set_input(&state, 0, "2147483647");
    if(sample == 8) { nm_set_input(&state, 0, "-5"); nm_set_input(&state, 1, "0"); }
    if(sample == 3 || sample == 4) nm_set_operation(&state, NM_SUB);
    selected = 0; result_view = false;
}

/* Modal settings leave the complete problem untouched on cancellation/error. */
static bool choose_width(void)
{
    char digits[3] = "";
    unsigned length = 0;
    bool bad = false;
    for(;;) {
        dclear(C_WHITE); info_title("Binary width: 1-32 bits", ACCENT);
        dprint(10, 40, C_DARK, "Current: %u bits", state.width);
        dprint(10, 70, C_BLACK, "New width: %s_", digits);
        dtext(10, 104, C_DARK, "Type 1-32, then EXE to apply.");
        dtext(10, 127, C_DARK, "DEL removes | AC clears | EXIT cancels");
        dtext(10, 150, C_DARK, "Narrower: keep low bits; warn if changed.");
        if(bad) dtext(10, 179, C_RED, "Enter a whole number from 1 to 32.");
        fkey_button(1, "8 BIT"); fkey_button(2, "16 BIT");
        fkey_button(3, "32 BIT"); fkey_button(6, "APPLY");
        dupdate();
        int key = getkey().key;
        if(key == KEY_EXIT) return false;
        if(key == KEY_F1 || key == KEY_F2 || key == KEY_F3)
            return nm_set_width(&state, key == KEY_F1 ? 8 : key == KEY_F2 ? 16 : 32);
        if(key == KEY_DEL) { if(length) digits[--length] = '\0'; bad = false; continue; }
        if(key == KEY_ACON) { length = 0; digits[0] = '\0'; bad = false; continue; }
        if(key == KEY_EXE || key == KEY_F6) {
            unsigned width = 0;
            for(unsigned i = 0; i < length; i++) width = width * 10 + (unsigned)(digits[i] - '0');
            if(!length || width < 1 || width > 32) { bad = true; continue; }
            return nm_set_width(&state, width);
        }
        int digit = key_digit(key);
        if(digit >= 0 && digit <= 9) {
            if(length == 2) { bad = true; continue; }
            digits[length++] = (char)('0' + digit); digits[length] = '\0'; bad = false;
        }
    }
}

void numeric_run(void)
{
    static bool initialized;
    if(!initialized) { nm_init(&state); initialized = true; }
    for(;;) {
        draw();
        int key = getkey().key;
        if(key == KEY_EXIT) return;
        if(key == INFO_KEY) { samples(); continue; }
        if(key == KEY_F1) {
            nm_set_base(&state, state.base == 10 ? 16 : state.base == 16 ? 2 : 10);
            result_view = false; continue;
        }
        if(key == KEY_F2) {
            nm_set_operation(&state, state.operation == NM_ADD ? NM_SUB : NM_ADD);
            result_view = false; continue;
        }
        if(key == KEY_F3) { nm_set_signed(&state, !state.signed_mode); result_view = false; continue; }
        if(key == KEY_F4) {
            bool had_result = state.solved;
            if(choose_width() || (had_result && !state.solved)) result_view = false;
            continue;
        }
        if(key == KEY_F5) { result_view = !result_view; continue; }
        if(key == KEY_F6 || key == KEY_EXE) {
            result_view = nm_solve(&state);
            if(!result_view) selected = state.operand_valid[0] ? 1 : 0;
            continue;
        }
        if(key == KEY_UP || key == KEY_DOWN) { selected ^= 1; result_view = false; continue; }
        if(key == KEY_ACON) { nm_clear(&state, selected); result_view = false; continue; }
        if(key == KEY_DEL) { nm_backspace(&state, selected); result_view = false; continue; }
        if(key == KEY_NEG || key == KEY_SUB) { nm_append(&state, selected, '-'); result_view = false; continue; }
        int digit = key_hex(key);
        if(digit >= 0) {
            nm_append(&state, selected, "0123456789ABCDEF"[digit]);
            result_view = false;
        }
    }
}
