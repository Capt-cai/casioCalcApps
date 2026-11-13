/* ECEN21 native views. Gray-map layout/group-edge ideas reuse upstream
 * Logic by Jose Rodrigues under the repository MIT license; Logic is intact. */
#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdio.h>
#include <string.h>

#include "solver.h"
#include "boolean_ui.h"
#include "info.h"
#include "i18n.h"
#include "keys.h"
#include "utils.h"

#define ACCENT C_RGB(2, 12, 17)
#define GROUP_COLOR C_RGB(30, 13, 2)
#define HIGHLIGHT C_RGB(26, 29, 31)

enum { EDIT, TABLE, KMAP, RESULT, GROUPS };
static bs_model model;
static unsigned view, table_row, map_row, map_col, group, result_page;
static eb_form form = EB_SOP;

static char const *source_name(void)
{
    return model.source == BS_EXPRESSION ? "EXPR" : "MANUAL";
}

static void source_line(void)
{
    dprint(10, 26, C_DARK, "%u vars | %s | %s", model.variables,
        source_name(), form == EB_SOP ? "SOP" : "POS");
}

static size_t next_line(char const *text, size_t start)
{
    size_t end = start, boundary = start;
    unsigned depth = 0;
    while(text[end] && end - start < 46) {
        if(text[end] == '(') depth++;
        if(text[end] == ')' && depth) depth--;
        if(!depth && (text[end] == '+' || text[end] == '*')) boundary = end + 1;
        end++;
    }
    return text[end] && boundary > start ? boundary : end;
}

static unsigned line_count(char const *text)
{
    unsigned lines = 0;
    for(size_t offset = 0; text[offset]; lines++) offset = next_line(text, offset);
    return lines ? lines : 1;
}

/* Every character is retained; pages avoid clipping long POS expressions. */
static void wrapped(char const *text, int y, unsigned first, unsigned count,
    int color)
{
    size_t offset = 0;
    for(unsigned line = 0; text[offset] && line < first + count; line++) {
        size_t end = next_line(text, offset), n = end - offset;
        if(line < first) { offset = end; continue; }
        char part[47];
        memcpy(part, text + offset, n);
        part[n] = '\0';
        dtext(10, y + (int)(line - first) * 15, color, part);
        offset = end;
    }
}

static void unsolved(void)
{
    bool error = !model.valid || model.status == BS_ERROR;
    dtext(10, 95, error ? C_RED : ACCENT,
        !error ? "Press F6 to compute SOP and POS." :
        "Input/calculation error. Return to EDIT (F1).");
    if(error) wrapped(model.error, 116, 0, 2, C_RED);
}

static void draw_editor(void)
{
    source_line();
    dtext(10, 44, ACCENT, "Expression (up to 79 characters)");
    drect_border(10, 56, 384, 101, C_WHITE, 1,
        model.valid ? ACCENT : C_RED);
    for(unsigned line = 0; line < 2; line++) {
        size_t length = strlen(model.expr), offset = line * 46;
        if(offset > length) break;
        size_t n = length - offset;
        if(n > 46) n = 46;
        char part[47];
        memcpy(part, model.expr + offset, n); part[n] = '\0';
        dtext(16, 64 + (int)line * 18, C_BLACK, part);
    }
    char prefix[47];
    unsigned col = model.cursor % 46, line = model.cursor / 46;
    memcpy(prefix, model.expr + line * 46, col); prefix[col] = '\0';
    int width = 0;
    dsize(prefix, NULL, &width, NULL);
    drect(16 + width, 64 + (int)line * 18,
        17 + width, 74 + (int)line * 18, ACCENT);
    if(!model.valid || model.status == BS_ERROR) {
        dprint(10, 108, C_RED, "Error at character %u:", model.error_pos + 1);
        wrapped(model.error, 122, 0, 2, C_RED);
    }
    else dtext(10, 112, ACCENT, model.source == BS_TABLE ?
        "Manual table active; saved text is inactive." : "Expression supplies the truth table.");
    dtext(10, 155, C_DARK, "X/log/ln/sin = A/B/C/D; 0/1 constants");
    dtext(10, 170, C_DARK, "+ OR  x AND  ^ XOR  / NOT  . postfix NOT");
    dtext(10, 190, C_DARK, model.source == BS_TABLE ?
        "Typing or F5 replaces manual data with text." :
        "Arrows: cursor | EXE solve | OPTN samples");
}

static void draw_table(void)
{
    source_line();
    if(!model.valid) { unsolved(); return; }
    unsigned rows = 1u << model.variables, per = rows > 8 ? 8 : rows;
    for(unsigned r = 0; r < rows; r++) {
        unsigned block = r / per, local = r % per;
        int x = 10 + (int)block * 198;
        int y = 62 + (int)local * 15;
        if(!local) {
            dtext(x, 45, C_DARK, "#");
            for(unsigned v = 0; v < model.variables; v++)
                dprint(x + 35 + (int)v * 23, 45, ACCENT, "%c", 'A' + (int)v);
            dtext(x + 35 + (int)model.variables * 23, 45, ACCENT, "F");
        }
        if(r == table_row) drect(x - 2, y - 1, x + 182, y + 11, HIGHLIGHT);
        dprint(x, y, C_DARK, "%u", r);
        for(unsigned v = 0; v < model.variables; v++)
            dprint(x + 35 + (int)v * 23, y, C_BLACK, "%u",
                (r >> (model.variables - 1 - v)) & 1u);
        int value = bs_row_value(&model, r);
        dtext(x + 35 + (int)model.variables * 23, y,
            value == 2 ? GROUP_COLOR : value ? ACCENT : C_DARK,
            value == 2 ? "X" : value ? "1" : "0");
    }
    dtext(10, 185, C_DARK, "EXE: 0 > 1 > X > 0 | arrows select");
    dtext(10, 196, C_DARK, model.source == BS_TABLE ?
        "F5 vars: keep low rows; discard high rows" :
        "F5 vars: re-evaluate expression (2-4)");
}

static bool cube_has(eb_cube cube, unsigned row)
{
    return (row & cube.fixed) == cube.bits;
}

static unsigned group_count(void)
{
    return model.status == BS_SOLVED ? model.result[form].count : 0;
}

static void term_text(char *text, size_t size)
{
    eb_result one = {.variables = (uint8_t)model.variables, .count = 1, .form = form};
    one.cubes[0] = model.result[form].cubes[group];
    unsigned fixed = one.cubes[0].fixed;
    while(fixed) { one.literals += fixed & 1u; fixed >>= 1; }
    eb_format(&one, text, size);
}

static void draw_map(void)
{
    source_line();
    if(!model.valid) { unsolved(); return; }
    unsigned rows = model.variables == 4 ? 4 : 2;
    unsigned cols = model.variables == 2 ? 2 : 4;
    int const x0 = 190, y0 = 52, cw = 44, ch = 30;
    unsigned const gray[] = {0, 1, 3, 2};
    dtext(145, 38, ACCENT, model.variables == 4 ? "AB/CD" :
        model.variables == 3 ? "A/BC" : "A/B");
    for(unsigned c = 0; c < cols; c++) {
        if(cols == 2) dprint(x0 + (int)c * cw + 18, 38, C_DARK, "%u", gray[c]);
        else dprint(x0 + (int)c * cw + 14, 38, C_DARK, "%u%u", gray[c] >> 1, gray[c] & 1);
    }
    for(unsigned r = 0; r < rows; r++) {
        if(rows == 2) dprint(177, y0 + (int)r * ch + 10, C_DARK, "%u", gray[r]);
        else dprint(164, y0 + (int)r * ch + 10, C_DARK, "%u%u", gray[r] >> 1, gray[r] & 1);
        for(unsigned c = 0; c < cols; c++) {
            unsigned index = bs_kmap_row(model.variables, r, c);
            int value = bs_row_value(&model, index);
            int x = x0 + (int)c * cw, y = y0 + (int)r * ch;
            int bg = value == 2 ? C_RGB(31, 28, 21) : value ? C_RGB(25, 30, 27) : C_WHITE;
            drect_border(x, y, x + cw, y + ch, bg, 1, C_LIGHT);
            dtext(x + 17, y + 4, value == 2 ? GROUP_COLOR : ACCENT,
                value == 2 ? "X" : value ? "1" : "0");
            dprint(x + 3, y + 18, C_DARK, "%u", index);
            if(group_count() && cube_has(model.result[form].cubes[group], index)) {
                eb_cube cube = model.result[form].cubes[group];
                unsigned up = bs_kmap_row(model.variables, (r + rows - 1) % rows, c);
                unsigned down = bs_kmap_row(model.variables, (r + 1) % rows, c);
                unsigned left = bs_kmap_row(model.variables, r, (c + cols - 1) % cols);
                unsigned right = bs_kmap_row(model.variables, r, (c + 1) % cols);
                if(!r || !cube_has(cube, up)) drect(x + 2, y + 2, x + cw - 2, y + 3, GROUP_COLOR);
                if(r + 1 == rows || !cube_has(cube, down)) drect(x + 2, y + ch - 3, x + cw - 2, y + ch - 2, GROUP_COLOR);
                if(!c || !cube_has(cube, left)) drect(x + 2, y + 2, x + 3, y + ch - 2, GROUP_COLOR);
                if(c + 1 == cols || !cube_has(cube, right)) drect(x + cw - 3, y + 2, x + cw - 2, y + ch - 2, GROUP_COLOR);
            }
            if(r == map_row && c == map_col)
                drect_border(x + 5, y + 1, x + cw - 5, y + ch - 1, C_NONE, 1, ACCENT);
        }
    }
    if(model.status == BS_SOLVED) {
        unsigned count = group_count();
        dprint(10, 53, ACCENT, "Group %u/%u", count ? group + 1 : 0, count);
        dtext(10, 72, C_DARK, form == EB_SOP ? "Groups cover 1/X" : "Groups cover 0/X");
        if(count) {
            char term[32]; term_text(term, sizeof term);
            dtext(10, 94, GROUP_COLOR, term);
        }
        else dtext(10, 94, C_DARK, "No groups needed");
        dtext(10, 127, C_DARK, "Orange: group");
        dtext(10, 142, C_DARK, "Blue: cursor");
    }
    else {
        dtext(10, 59, C_DARK, "No current groups");
        dtext(10, 80, ACCENT, "F6 computes them");
    }
    dtext(10, 185, C_DARK, "+/-: select group | EXE: 0 > 1 > X");
    dtext(10, 196, C_DARK, "F5: group details | F4: full result");
}

static void draw_result(void)
{
    source_line();
    if(model.status != BS_SOLVED) { unsolved(); return; }
    eb_result const *result = &model.result[form];
    unsigned pages = (line_count(model.formatted[form]) + 5) / 6;
    if(result_page >= pages) result_page = pages - 1;
    dprint(10, 45, ACCENT, "Exact %s: %u terms, %u literals", form == EB_SOP ? "SOP" : "POS",
        (unsigned)result->count, (unsigned)result->literals);
    wrapped(model.formatted[form], 65, result_page * 6, 6, C_BLACK);
    dprint(10, 165, ACCENT, "%u cared rows checked (both forms)", model.checked_rows);
    dprint(10, 181, C_DARK, "Page %u/%u | F5 explains groups", result_page + 1, pages);
    dtext(10, 196, C_DARK, "UP/DOWN page | F4 swaps SOP/POS");
}

static void rows_text(eb_cube cube, bool only_dc, char *text, size_t size)
{
    size_t n = 0;
    text[0] = '\0';
    for(unsigned r = 0; r < (1u << model.variables); r++) {
        if(!cube_has(cube, r) || (only_dc && !(model.dc & (1u << r)))) continue;
        int wrote = snprintf(text + n, size - n, "%s%u", n ? "," : "", r);
        if(wrote < 0 || (size_t)wrote >= size - n) break;
        n += (size_t)wrote;
    }
    if(!n) snprintf(text, size, "none");
}

static void draw_groups(void)
{
    source_line();
    if(model.status != BS_SOLVED) { unsolved(); return; }
    unsigned count = group_count();
    if(!count) {
        dtext(10, 60, ACCENT, "Empty cover: no groups are required.");
        dprint(10, 84, C_BLACK, "Constant %s = %s", form == EB_SOP ? "SOP" : "POS", model.formatted[form]);
        dtext(10, 120, C_DARK, "Don't-care rows impose no requirement.");
        return;
    }
    char text[96];
    term_text(text, sizeof text);
    dprint(10, 46, ACCENT, "Group %u/%u -> %s", group + 1, count, text);
    eb_cube cube = model.result[form].cubes[group];
    size_t n = 0;
    for(unsigned v = 0; v < model.variables; v++) {
        unsigned bit = 1u << (model.variables - v - 1);
        n += (size_t)snprintf(text + n, sizeof text - n, "%c=%c  ", 'A' + (int)v,
            !(cube.fixed & bit) ? '*' : (cube.bits & bit) ? '1' : '0');
    }
    dtext(10, 68, C_BLACK, text);
    dtext(10, 89, C_DARK, "Covered row numbers:");
    rows_text(cube, false, text, sizeof text); wrapped(text, 104, 0, 2, C_BLACK);
    dtext(10, 135, C_DARK, "Don't-care rows included:");
    rows_text(cube, true, text, sizeof text); wrapped(text, 150, 0, 2, GROUP_COLOR);
    dtext(10, 181, ACCENT, form == EB_SOP ?
        "Fixed variables remain; * variables drop." : "POS: complement the zero-group cube.");
    dtext(10, 196, C_DARK, "UP/DOWN: every group | F5: K-map");
}

static void draw(void)
{
    static char const *titles[] = {"ECEN21 - Expression", "ECEN21 - Truth table",
        "ECEN21 - K-map", "ECEN21 - Exact result", "ECEN21 - Group steps"};
    unsigned count = group_count();
    if(group >= count) group = 0;
    dclear(C_WHITE);
    info_title(titles[view], ACCENT);
    if(view == EDIT) draw_editor();
    else if(view == TABLE) draw_table();
    else if(view == KMAP) draw_map();
    else if(view == RESULT) draw_result();
    else draw_groups();
    fkey_button_color(1, "EDIT", view == EDIT ? ACCENT : C_BLACK);
    fkey_button_color(2, "TABLE", view == TABLE ? ACCENT : C_BLACK);
    fkey_button_color(3, "KMAP", view == KMAP ? ACCENT : C_BLACK);
    fkey_button(4, view == RESULT ? (form == EB_SOP ? "TO POS" : "TO SOP") : "RESULT");
    fkey_button(5, view == EDIT ? "USE EX" : view == TABLE ? "VARS" : view == GROUPS ? "KMAP" : "GROUP");
    fkey_button_color(6, "SOLVE", ACCENT);
    dupdate();
}

static void sample_menu(void)
{
    dclear(C_WHITE);
    info_title("Samples / About ECEN21", ACCENT);
    dtext(10, 29, C_DARK, "Choose a sample: replaces current work.");
    dtext(10, 53, C_BLACK, "1  AB+C (4 variables)");
    dtext(10, 73, C_BLACK, "2  Exact-cover counterexample (007E)");
    dtext(10, 93, C_BLACK, "3  Don't-care example (2 variables)");
    dtext(10, 113, C_BLACK, "4  A^B^C^D (parity / long results)");
    dtext(10, 133, C_BLACK, "5  Constant 0");
    dtext(10, 153, C_BLACK, "6  Constant 1");
    dtext(10, 179, C_DARK, "Other keys return without changing data.");
    dtext(10, 197, C_DARK, "casioCalcApps (MIT) - Jose Rodrigues");
    dupdate();
    int choice = key_digit(getkey().key);
    if(choice < 1 || choice > 6) return;
    bs_init(&model);
    if(choice == 2) bs_set_expression(&model, "A'(B^C+C^D)");
    if(choice == 3) {
        bs_set_expression(&model, "0");
        bs_set_variables(&model, 2);
        bs_set_expression(&model, "A'B");
        bs_cycle_row(&model, 0); bs_cycle_row(&model, 0);
    }
    if(choice == 4) bs_set_expression(&model, "A^B^C^D");
    if(choice == 5) bs_set_expression(&model, "0");
    if(choice == 6) bs_set_expression(&model, "1");
    view = choice == 3 ? TABLE : EDIT;
    table_row = map_row = map_col = group = result_page = 0;
    form = EB_SOP;
}

static char expression_key(int key)
{
    switch(key) {
        case KEY_XOT: return 'A'; case KEY_LOG: return 'B';
        case KEY_LN: return 'C'; case KEY_SIN: return 'D';
        case KEY_0: return '0'; case KEY_1: return '1';
        case KEY_ADD: return '+'; case KEY_MUL: return '*';
        case KEY_POWER: return '^'; case KEY_DIV: return '!';
        case KEY_DOT: return '\''; case KEY_LEFTP: return '(';
        case KEY_RIGHTP: return ')'; default: return 0;
    }
}

void boolean_run(void)
{
    static bool initialized;
    if(!initialized) { bs_init(&model); initialized = true; }
    for(;;) {
        draw();
        int key = getkey().key;
        if(key == KEY_EXIT) return;
        if(key == INFO_KEY) { sample_menu(); continue; }
        if(key == KEY_F1) { view = EDIT; continue; }
        if(key == KEY_F2) { view = TABLE; continue; }
        if(key == KEY_F3) { view = KMAP; continue; }
        if(key == KEY_F4) {
            if(view == RESULT) { form = form == EB_SOP ? EB_POS : EB_SOP; group = result_page = 0; }
            view = RESULT; continue;
        }
        if(key == KEY_F6 || (view == EDIT && key == KEY_EXE)) {
            if(bs_solve(&model)) { if(view == EDIT) view = RESULT; }
            else view = EDIT;
            group = result_page = 0; continue;
        }
        if(key == KEY_F5) {
            if(view == EDIT) bs_use_expression(&model);
            else if(view == TABLE) {
                bs_set_variables(&model, model.variables == 4 ? 2 : model.variables + 1);
                table_row = map_row = map_col = group = result_page = 0;
            }
            else if(view == GROUPS) view = KMAP;
            else view = GROUPS;
            continue;
        }
        if(view == EDIT) {
            if(key == KEY_LEFT && model.cursor) model.cursor--;
            else if(key == KEY_RIGHT && model.cursor < strlen(model.expr)) model.cursor++;
            else if(key == KEY_DEL) bs_backspace(&model);
            else if(key == KEY_ACON) bs_clear(&model);
            else { char c = expression_key(key); if(c) bs_insert(&model, c); }
        }
        else if(view == TABLE) {
            unsigned rows = 1u << model.variables;
            if(key == KEY_UP) table_row = (table_row + rows - 1) % rows;
            if(key == KEY_DOWN) table_row = (table_row + 1) % rows;
            if((key == KEY_LEFT || key == KEY_RIGHT) && rows == 16) table_row = (table_row + 8) % rows;
            if(key == KEY_EXE) { bs_cycle_row(&model, table_row); group = result_page = 0; }
        }
        else if(view == KMAP) {
            unsigned rows = model.variables == 4 ? 4 : 2, cols = model.variables == 2 ? 2 : 4;
            if(key == KEY_UP) map_row = (map_row + rows - 1) % rows;
            if(key == KEY_DOWN) map_row = (map_row + 1) % rows;
            if(key == KEY_LEFT) map_col = (map_col + cols - 1) % cols;
            if(key == KEY_RIGHT) map_col = (map_col + 1) % cols;
            if(key == KEY_EXE) { bs_cycle_row(&model, bs_kmap_row(model.variables, map_row, map_col)); group = result_page = 0; }
            unsigned count = group_count();
            if(count && key == KEY_ADD) group = (group + 1) % count;
            if(count && key == KEY_SUB) group = (group + count - 1) % count;
        }
        else if(view == RESULT && model.status == BS_SOLVED) {
            unsigned pages = (line_count(model.formatted[form]) + 5) / 6;
            if(key == KEY_DOWN) result_page = (result_page + 1) % pages;
            if(key == KEY_UP) result_page = (result_page + pages - 1) % pages;
        }
        else if(view == GROUPS && group_count()) {
            unsigned count = group_count();
            if(key == KEY_DOWN || key == KEY_RIGHT) group = (group + 1) % count;
            if(key == KEY_UP || key == KEY_LEFT) group = (group + count - 1) % count;
        }
    }
}
