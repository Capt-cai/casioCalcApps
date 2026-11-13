#include <gint/display.h>
#include <gint/keyboard.h>
#include "boolean_ui.h"
#include "numeric_ui.h"
#include "flipflop_ui.h"
#include "sequential_ui.h"
#include "timing_ui.h"
#include "combinational_ui.h"
#include "hdl_ui.h"
#include "info.h"
#include "i18n.h"
#include "utils.h"

#define ACCENT C_RGB(2, 12, 17)

int main(void)
{
    lang_init(); screen_init(); app_name = "ECEN21 Toolkit";
    unsigned selected = 0;
    for(;;) {
        dclear(C_WHITE);
        info_title("ECEN21 Toolkit", ACCENT);
        drect(10, 24 + (int)selected * 26, 384, 49 + (int)selected * 26,
            C_RGB(26, 29, 31));
        dtext(18, 27, C_BLACK, "1  Boolean solver");
        dtext(18, 40, C_DARK, "Expressions, tables, K-map, SOP/POS");
        dtext(18, 53, C_BLACK, "2  Binary / signed numbers");
        dtext(18, 66, C_DARK, "Custom1-32-bit conversion and + / -");
        dtext(18, 79, C_BLACK, "3  Flip-flops (SR / D / JK / T)");
        dtext(18, 92, C_DARK, "Tables, single steps, 16-step history");
        dtext(18, 105, C_BLACK, "4  Counter / sequential design");
        dtext(18, 118, C_DARK, "Sequence -> equations -> circuit / reverse");
        dtext(18, 131, C_BLACK, "5  Timing / connected circuits");
        dtext(18, 144, C_DARK, "Waves, latches, reset, event explanation");
        dtext(18, 157, C_BLACK, "6  Combinational circuit design");
        dtext(18, 170, C_DARK, "Adders, MUX, encoders, seven-segment");
        dtext(18, 183, C_BLACK, "7  Verilog code generator");
        dtext(18, 196, C_DARK, "Parameters -> code -> USB .v text export");
        fkey_button_color(1, "BOOL", ACCENT);
        fkey_button_color(2, "BINARY", ACCENT);
        fkey_button_color(3, "FLIP", ACCENT);
        fkey_button_color(4, "SEQ", ACCENT);
        fkey_button_color(5, "TIME", ACCENT);
        fkey_button_color(6, "COMB", ACCENT);
        dupdate();
        int key = getkey().key;
        if(key == KEY_EXIT) return 1;
        if(key == KEY_UP) selected = (selected + 6) % 7;
        if(key == KEY_DOWN) selected = (selected + 1) % 7;
        if(key == KEY_1 || key == KEY_F1) { selected = 0; boolean_run(); }
        else if(key == KEY_2 || key == KEY_F2) { selected = 1; numeric_run(); }
        else if(key == KEY_3 || key == KEY_F3) { selected = 2; flipflop_run(); }
        else if(key == KEY_4 || key == KEY_F4) { selected = 3; sequential_run(); }
        else if(key == KEY_5 || key == KEY_F5) { selected = 4; timing_run(); }
        else if(key == KEY_6 || key == KEY_F6) { selected = 5; combinational_run(); }
        else if(key == KEY_7) { selected = 6; hdl_run(); }
        else if(key == KEY_EXE) {
            if(selected == 6) hdl_run();
            else if(selected == 5) combinational_run();
            else if(selected == 4) timing_run();
            else if(selected == 3) sequential_run();
            else if(selected == 2) flipflop_run();
            else if(selected == 1) numeric_run(); else boolean_run();
        }
        else if(key == INFO_KEY) {
            dclear(C_WHITE);
            info_title("About ECEN21 Toolkit", ACCENT);
            dtext(12, 48, C_BLACK, "Offline logic, sequential and timing tools");
            dtext(12, 78, C_DARK, "Based on casioCalcApps (MIT)");
            dtext(12, 100, C_DARK, "Original project: Jose Rodrigues");
            dtext(12, 158, ACCENT, "Press any key to return.");
            dupdate(); getkey();
        }
    }
}
