#include <stdio.h>
#include <stdlib.h>
#include <menu.h>
#include <string.h>
#include <linux/hid.h>
#include "helpers.h"
#include "logitech_g300.h"

#define NAME "Logitech G300/G300S"
#define VID 0x046D
#define PID 0xC246

/** The mouse has two interfaces:
 *   0 -> Mouse
 *   1 -> Keyboard (important one)
 **/
#define IFACE 1

#define GET_MODE_HEX(mode_index) (mode_index + 0xf3)
#define GET_MODE_IND(mode_hex) (mode_hex - 0xf3)

#define MODE_DATA_LEN 35

#define STR_LEN 64

#define DPI_VAL(dpi) (!(dpi) ? 4000 : (dpi)*250)
#define DPI_SHIFT(b) b & 0x40 ? "OFF" : "ON"


char *scheme_ascii[24] = {
    "           \" ",
    "           \" ",
    "  ,------. \" .------,",
    " /'      \\;;;/      '\\",
    "/_        ';'        _\\",
    "[5|        |        |7]",
    "[_|        |        |_]",
    "[4|       | |       |6]",
    "[_|       |_|       |_]",
    ";\\         |         /;",
    " |         |         |",
    " |        [9]        |",
    " |        [8]        |",
    " |         |         |",
    "/;                   ;\\",
    "|                     |",
    "|                     |",
    "|;                   ;|",
    ";\\                   /;",
    " \\\\                 //",
    "  \\\\               //",
    "   \\'._         _.'/",
    "    '\\\\__.....__//'",
    "        '-----' "
};


const char *s_color[] = {
     "black"
    ,"red"
    ,"green"
    ,"yellow"
    ,"blue"
    ,"magenta"
    ,"cyan"
    ,"white"
};

const char *s_buttons[] = {
     "NONE"
    ,"Button1"
    ,"Button2"
    ,"Button3"
    ,"Button6"
    ,"Button7"
    ,"Button8"
    ,"Button9"
    ,"Button10"
    ,"Button11"
    ,"DPIUp"
    ,"DPIDown"
    ,"DPICycle"
    ,"ModeSwitch"
    ,"DPIShift"
    ,"DPIDefault"
};

// Turns out these are likely HID standard codes!
// ( https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf )
const char *s_keys[] = {
     "NONE"
    ,"UNKNOWN:01" // 01 ==   1 // "HID: Keyboard Err: Rollover - not a key
    ,"UNKNOWN:02" // 02 ==   2 // "HID: Keyboard Err: POST Fail - not a key
    ,"UNKNOWN:03" // 03 ==   3 // "HID: Keyboard Err: Undefined - not a key
    ,"A"          // 04 ==   4
    ,"B"
    ,"C"
    ,"D"
    ,"E"
    ,"F"
    ,"G"
    ,"H"
    ,"I"
    ,"J"
    ,"K"
    ,"L"
    ,"M"
    ,"N"
    ,"O"
    ,"P"
    ,"Q"
    ,"R"
    ,"S"
    ,"T"
    ,"U"
    ,"V"
    ,"W"
    ,"X"
    ,"Y"
    ,"Z"          // 1D ==  29
    ,"1"          // 1E ==  30
    ,"2"
    ,"3"
    ,"4"
    ,"5"
    ,"6"
    ,"7"
    ,"8"
    ,"9"
    ,"0"           // 27 ==  39
    ,"Enter"       // 28 ==  40
    ,"Escape"      // 29 ==  41
    ,"Backspace"   // 2a ==  42
    ,"Tab"         // 2b ==  43
    ,"Space"       // 2c ==  44
    ,"-"           // 2d ==  45
    ,"="           // 2e ==  46
    ,"["           // 2f ==  47
    ,"]"           // 30 ==  48
    ,"\\"          // 31 ==  49
    ,"NonUS#"      // 32 ==  50
    ,";"           // 33 ==  51
    ,"'"           // 34 ==  52
    ,"`"           // 35 ==  53
    ,","           // 36 ==  54
    ,"."           // 37 ==  55
    ,"/"           // 38 ==  56
    ,"CapsLock"    // 39 ==  57
    ,"F1"          // 3a ==  58
    ,"F2"          // 3b ==  59
    ,"F3"          // 3c ==  60
    ,"F4"          // 3d ==  61
    ,"F5"          // 3e ==  62
    ,"F6"          // 3f ==  63
    ,"F7"          // 40 ==  64
    ,"F8"          // 41 ==  65
    ,"F9"          // 42 ==  66
    ,"F10"         // 43 ==  67
    ,"F11"         // 44 ==  68
    ,"F12"         // 45 ==  69
    ,"PrintScreen" // 46 ==  70
    ,"ScrollLock"  // 47 ==  71
    ,"Pause"       // 48 ==  72
    ,"Insert"      // 49 ==  73
    ,"Home"        // 4a ==  74
    ,"PageUp"      // 4b ==  75
    ,"Delete"      // 4c ==  76
    ,"End"         // 4d ==  77
    ,"PageDown"    // 4e ==  78
    ,"Right"       // 4f ==  79
    ,"Left"        // 50 ==  80
    ,"Down"        // 51 ==  81
    ,"Up"          // 52 ==  82
    ,"NumLock"     // 53 ==  83
    ,"Num/"        // 54 ==  84
    ,"Num*"        // 55 ==  85
    ,"Num-"        // 56 ==  86
    ,"Num+"        // 57 ==  87
    ,"NumEnter"    // 58 ==  88
    ,"Num1"        // 59 ==  89
    ,"Num2"        // 5a ==  90
    ,"Num3"        // 5b ==  91
    ,"Num4"        // 5c ==  92
    ,"Num5"        // 5d ==  93
    ,"Num6"        // 5e ==  94
    ,"Num7"        // 5f ==  95
    ,"Num8"        // 60 ==  96
    ,"Num9"        // 61 ==  97
    ,"Num0"        // 62 ==  98
    ,"Num."        // 63 ==  99
    ,"NonUS\\"     // 64 == 100
    ,"Application" // 65 == 101
    ,"Power"       // 66 == 102
    ,"Num="        // 67 == 103
    ,"F13"         // 68 == 104
    ,"F14"         // 69 == 105
    ,"F15"         // 6a == 106
    ,"F16"         // 6b == 107
    ,"F17"         // 6c == 108
    ,"F18"         // 6d == 109
    ,"F19"         // 6e == 110
    ,"F20"         // 6f == 111
    ,"F21"         // 70 == 112
    ,"F22"         // 71 == 113
    ,"F23"         // 72 == 114
    ,"F24"         // 73 == 115
    ,"Execute"     // 74 == 116
    ,"Help"        // 75 == 117
    ,"Menu"        // 76 == 118
    ,"Select"      // 77 == 119
    ,"Stop"        // 78 == 120
    ,"Again"       // 79 == 121
    ,"Undo"        // 7a == 122
    ,"Cut"         // 7b == 123
    ,"Copy"        // 7c == 124
    ,"Paste"       // 7d == 125
    ,"Find"        // 7e == 126
    ,"Mute"        // 7f == 127
    ,"VolumeUp"    // 80 == 128
    ,"VolumeDown"  // 81 == 129
    ,"UNKNOWN:82"  // 82 == 130 // Locking CapsLock but legacy so not defining
    ,"UNKNOWN:83"  // 83 == 131 // Locking CapsLock but legacy so not defining
    ,"UNKNOWN:84"  // 84 == 132 // Locking CapsLock but legacy so not defining
    ,"Num,"        // 85 == 133 // Brazillian keypad period (.)?
    ,"AS400Num="   // 86 == 134 // Keypad Equal Sign on AS/400 keyboards
    ,"UNKNOWN:87"  // 87 == 135 // International 1?
    ,"UNKNOWN:88"  // 88 == 136 // International 2?
    ,"UNKNOWN:89"  // 89 == 137 // International 3?
    ,"UNKNOWN:8a"  // 8a == 138 // International 4?
    ,"UNKNOWN:8b"  // 8b == 139 // International 5?
    ,"UNKNOWN:8c"  // 8c == 140 // International 6?
    ,"UNKNOWN:8d"  // 8d == 141 // International 7?
    ,"UNKNOWN:8e"  // 8e == 142 // International 8?
    ,"UNKNOWN:8f"  // 8f == 143 // International 9?
    ,"UNKNOWN:90"  // 90 == 144 // LANG1 - Hangul/English toggle - Korean?
    ,"UNKNOWN:91"  // 91 == 145 // LANG2 - Hanja conversion key - Korean?
    ,"UNKNOWN:92"  // 92 == 146 // LANG3 - Katakana key - Japanese?
    ,"UNKNOWN:93"  // 93 == 147 // LANG4 - Hiragana key - Japanese?
    ,"UNKNOWN:94"  // 94 == 148 // LANG5 - Zenkaku/Hankaku key - Japanese?
    ,"UNKNOWN:95"  // 95 == 149 // LANG6 - Reserved?
    ,"UNKNOWN:96"  // 96 == 150 // LANG7 - Reserved?
    ,"UNKNOWN:97"  // 97 == 151 // LANG8 - Reserved?
    ,"UNKNOWN:98"  // 98 == 152 // LANG9 - Reserved?
    ,"UNKNOWN:99"  // 99 == 153 // Alternate Erase (Erase-Eaze(tm))?
    ,"SysReq"      // 9a == 154 // SysReq/Attention
    ,"Cancel"      // 9b == 155
    ,"Clear"       // 9c == 156
    ,"Prior"       // 9d == 157
    ,"Return"      // 9e == 158
    ,"Separator"   // 9f == 159
    ,"Out"         // a0 == 160
    ,"Oper"        // a1 == 161
    ,"ClearAgain"  // a2 == 162
    ,"CrSelProps"  // a3 == 163
    ,"ExSel"       // a4 == 164
    ,"UNKNOWN:a5"  // a5 == 165 // Reserved
    ,"UNKNOWN:a6"  // a6 == 166 // Reserved
    ,"UNKNOWN:a7"  // a7 == 167 // Reserved
    ,"UNKNOWN:a8"  // a8 == 168 // Reserved
    ,"UNKNOWN:a9"  // a9 == 169 // Reserved
    ,"UNKNOWN:aa"  // aa == 170 // Reserved
    ,"UNKNOWN:ab"  // ab == 171 // Reserved
    ,"UNKNOWN:ac"  // ac == 172 // Reserved
    ,"UNKNOWN:ad"  // ad == 173 // Reserved
    ,"UNKNOWN:ae"  // ae == 174 // Reserved
    ,"UNKNOWN:af"  // af == 175 // Reserved
    ,"Num00"       // b0 == 176
    ,"Num000"      // b1 == 177
    ,"Sep1000s"    // b2 == 178 // Thousands separator - locale specific?
    ,"SepDec"      // b3 == 179 // Decimal   separator - locale specific?
    ,"CurrUnit"    // b4 == 180 // Currency Unit       - locale specific?
    ,"CurrSubUnit" // b5 == 181 // Currency Sub-Unit   - locale specific?
    ,"Num("        // b6 == 182
    ,"Num)"        // b7 == 183
    ,"Num{"        // b8 == 184
    ,"Num}"        // b9 == 185
    ,"NumTab"      // ba == 186
    ,"NumBackspace"// bb == 187
    ,"NumA"        // bc == 188
    ,"NumB"        // bd == 189
    ,"NumC"        // be == 190
    ,"NumD"        // bf == 191
    ,"NumE"        // c0 == 192
    ,"NumF"        // c1 == 193
    ,"NumXOR"      // c2 == 194
    ,"Num^"        // c3 == 195
    ,"Num%"        // c4 == 196
    ,"Num<"        // c5 == 197
    ,"Num>"        // c6 == 198
    ,"Num&"        // c7 == 199
    ,"Num&&"       // c8 == 200
    ,"Num|"        // c9 == 201
    ,"Num||"       // ca == 202
    ,"Num:"        // cb == 203
    ,"Num#"        // cc == 204
    ,"NumSpace"    // cd == 205
    ,"Num@"        // ce == 206
    ,"Num!"        // cf == 207
    ,"NumMemStore" // d0 == 208
    ,"NumMemRecall"// d1 == 209
    ,"NumMemClear" // d2 == 210
    ,"NumMemAdd"   // d3 == 211
    ,"NumMemSub"   // d4 == 212
    ,"NumMemMul"   // d5 == 213
    ,"NumMemDiv"   // d6 == 214
    ,"NumPlusMinus"// d7 == 215
    ,"NumClear"    // d8 == 216
    ,"NumClearEntry"// d9 == 217
    ,"NumBinary"   // da == 218
    ,"NumOctal"    // db == 219
    ,"NumDecimal"  // dc == 220
    ,"NumHex"      // dd == 221
    ,"UNKNOWN:de"  // de == 222 // Reserved
    ,"UNKNOWN:df"  // df == 223 // Reserved
    ,"LeftCtrl"    // e0 == 224
    ,"LeftShift"   // e1 == 225
    ,"LeftAlt"     // e2 == 226
    ,"Super_L"     // e3 == 227 // Left GUI
    ,"RightCtrl"   // e4 == 228
    ,"RightShift"  // e5 == 229
    ,"RightAlt"    // e6 == 230
    ,"Super_R"     // e7 == 231 // Right GUI
    ,"UNKNOWN:e8"  // e8 == 232 // Reserved
    ,"UNKNOWN:e9"  // e9 == 233 // Reserved
    ,"UNKNOWN:ea"  // ea == 234 // Reserved
    ,"UNKNOWN:eb"  // eb == 235 // Reserved
    ,"UNKNOWN:ec"  // ec == 236 // Reserved
    ,"UNKNOWN:ed"  // ed == 237 // Reserved
    ,"UNKNOWN:ee"  // ee == 238 // Reserved
    ,"UNKNOWN:ef"  // ef == 239 // Reserved
    ,"UNKNOWN:f0"  // f0 == 240 // Reserved
    ,"UNKNOWN:f1"  // f1 == 241 // Reserved
    ,"UNKNOWN:f2"  // f2 == 242 // Reserved
    ,"UNKNOWN:f3"  // f3 == 243 // Reserved
    ,"UNKNOWN:f4"  // f4 == 244 // Reserved
    ,"UNKNOWN:f5"  // f5 == 245 // Reserved
    ,"UNKNOWN:f6"  // f6 == 246 // Reserved
    ,"UNKNOWN:f7"  // f7 == 247 // Reserved
    ,"UNKNOWN:f8"  // f8 == 248 // Reserved
    ,"UNKNOWN:f9"  // f9 == 249 // Reserved
    ,"UNKNOWN:fa"  // fa == 250 // Reserved
    ,"UNKNOWN:fb"  // fb == 251 // Reserved
    ,"UNKNOWN:fc"  // fc == 252 // Reserved
    ,"UNKNOWN:fd"  // fd == 253 // Reserved
    ,"UNKNOWN:fe"  // fe == 254 // Reserved
                   // ff = 255  // Reserved (and even if it weren't, it's not
                   //              used in the loops :P)
};



short g300_change_color(WINDOW *dev_win, u_char *mode_data);
short g300_change_dpi(WINDOW *dev_win, u_char *mode_data, short level);
void g300_toggle_dpi_shift(u_char *mode_data);
short g300_change_button(WINDOW *dev_win, u_char *mode_data, short button);
void g300_get_key_str(u_char *mode_data, char *str);
short g300_mode_menu(WINDOW *dev_win);
short g300_load_mode(libusb_device_handle *dev_handle, uint16_t mode, u_char *mode_data);
void g300_print_mode(WINDOW *dev_win, u_char *mode_data, int sel, bool highlight, bool modified);
void g300_cb(libusb_device_handle *dev_handle, WINDOW *dev_win);


Device Logitech_G300 = {
    NAME,
    MOUSE,
    VID,
    PID,
    g300_cb
};

void g300_cb(libusb_device_handle *dev_handle, WINDOW *dev_win) {
    int c, f, sel;
    bool highlight, modified;
    uint16_t mode;
    u_char mode_data[MODE_DATA_LEN];

    if (dev_handle == NULL) return;

    /* Mouse init */
    if (libusb_kernel_driver_active(dev_handle, IFACE) == 1) {
        /* Detach kernel driver if necesary */
        if (libusb_detach_kernel_driver(dev_handle, IFACE) != 0) {
            print_in_middle(dev_win, LINES/2-1, 0, COLS, "Error detaching kernel driver from device", COLOR_PAIR(1));
            print_in_middle(dev_win, LINES/2, 0, COLS, "Press \"q\" to exit...", COLOR_PAIR(1));
            wrefresh(dev_win);
            while((c = wgetch(dev_win)) != 'q');
            return;
        }
    }
    if (libusb_claim_interface(dev_handle, IFACE) != 0) {
        print_in_middle(dev_win, LINES/2-1, 0, COLS, "Error claiming device interface", COLOR_PAIR(1));
        print_in_middle(dev_win, LINES/2, 0, COLS, "Press \"q\" to exit...", COLOR_PAIR(1));
        wrefresh(dev_win);
        libusb_attach_kernel_driver(dev_handle, IFACE);
        while((c = wgetch(dev_win)) != 'q');
        return;
    }

    /* Get mode */
    f = g300_mode_menu(dev_win);
    if (f < 0) {
        /* Mouse deinit */
        libusb_release_interface(dev_handle, IFACE);
        libusb_attach_kernel_driver(dev_handle, IFACE);
        return;
    }
    mode = GET_MODE_HEX(f);
    f = g300_load_mode(dev_handle, mode, mode_data);
    if (f < 0) {
        print_in_middle(dev_win, LINES/2-1, 0, COLS, "Error loading mode from device", COLOR_PAIR(1));
        print_in_middle(dev_win, LINES/2, 0, COLS, "Press \"q\" to exit...", COLOR_PAIR(1));
        wrefresh(dev_win);
        /* Mouse deinit */
        libusb_release_interface(dev_handle, IFACE);
        libusb_attach_kernel_driver(dev_handle, IFACE);
        while((c = wgetch(dev_win)) != 'q');
        return;
    }

    /* Print mode */
    sel = 0;
    halfdelay(5);   /* To enable blink of selected menu item */
    highlight = TRUE;
    modified = FALSE;
    g300_print_mode(dev_win, mode_data, sel, highlight, modified);

    while((c = wgetch(dev_win)) != 'q') {
        highlight = !highlight;
        switch (c) {
            case 'j':
            case KEY_DOWN:
                sel = (sel+1) % 10;
                highlight = TRUE;
                break;

            case 'k':
            case KEY_UP:
                sel == 0 ? sel=9 : sel--;
                highlight = TRUE;
                break;

            case 'm':
                /* Get new mode */
                f = g300_mode_menu(dev_win);
                if (f < 0) break;
                mode = GET_MODE_HEX(f);
                f = g300_load_mode(dev_handle, mode, mode_data);
                if (f < 0) {
                    wclear(dev_win);
                    print_in_middle(dev_win, LINES/2-1, 0, COLS, "Error loading mode from device", COLOR_PAIR(1));
                    print_in_middle(dev_win, LINES/2, 0, COLS, "Press \"q\" to exit...", COLOR_PAIR(1));
                    wrefresh(dev_win);
                    /* Mouse deinit */
                    libusb_release_interface(dev_handle, IFACE);
                    libusb_attach_kernel_driver(dev_handle, IFACE);
                    while((c = wgetch(dev_win)) != 'q');
                    return;
                }
                break;
            
            case 's':
                // TODO Save...
                break;

            case 10:
                if (sel < 1) {
                    if (g300_change_color(dev_win, mode_data)) modified = TRUE;
                } else if (sel < 5) {
                    if (g300_change_dpi(dev_win, mode_data, sel - 1)) modified = TRUE;
                }
                else if (sel == 5) {
                    g300_toggle_dpi_shift(mode_data);
                    modified = TRUE;
                }
                else {
                    if (g300_change_button(dev_win, mode_data, sel - 6)) modified = TRUE;
                }
                break;

            case '?':
                // TODO Help...

            default:
                break;
        }
        /* Print mode */
        g300_print_mode(dev_win, mode_data, sel, highlight, modified);
    }
    cbreak();

    /* Mouse deinit */
    libusb_release_interface(dev_handle, IFACE);
    libusb_attach_kernel_driver(dev_handle, IFACE);

    return;
}



short g300_load_mode(libusb_device_handle *dev_handle, uint16_t mode, u_char *mode_data) {
    int f;

    if (dev_handle == NULL || mode_data == NULL) return ERR;
    if (mode != 0xf3 && mode != 0xf4 && mode != 0xf5) return ERR;

    f = libusb_control_transfer(
         dev_handle
        ,LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_IN
        ,HID_REQ_GET_REPORT
        ,0x0300|mode
        ,0x0001
        ,mode_data
        ,MODE_DATA_LEN
        ,1000
    );
    //usleep(10000);
    
    if (f < MODE_DATA_LEN) return ERR;
    return OK;
}


short g300_mode_menu(WINDOW *dev_win) {
    MENU *mode_select;
    ITEM *modes[4], *currentItem;
    WINDOW *mode_menu_win;
    int height, width, c;
    short i, k;

    if (dev_win == NULL) return -1;

    modes[0] = new_item("Mode 1", NULL);
    modes[1] = new_item("Mode 2", NULL);
    modes[2] = new_item("Mode 3", NULL);
    modes[3] = NULL;

    mode_select = new_menu(modes);
    getmaxyx(dev_win, height, width);
    mode_menu_win = newwin(7,20,height/2 - 3, width/2 - 10);
    keypad(mode_menu_win, TRUE);
    set_menu_win(mode_select, mode_menu_win);
    set_menu_sub(mode_select, derwin(mode_menu_win, 3, 11, 3, 4));
    set_menu_mark(mode_select, " > ");
    box(mode_menu_win, 0, 0);
    print_in_middle(mode_menu_win, 1, 0, 20, "Select a mode:", COLOR_PAIR(3));
	mvwaddch(mode_menu_win, 2, 0, ACS_LTEE);
	mvwhline(mode_menu_win, 2, 1, ACS_HLINE, 18);
	mvwaddch(mode_menu_win, 2, 19, ACS_RTEE);
	wrefresh(dev_win);
    post_menu(mode_select);
    wrefresh(mode_menu_win);

    do {
        c = wgetch(mode_menu_win);
        switch(c) {
            case 'j':
            case KEY_DOWN:
				menu_driver(mode_select, REQ_DOWN_ITEM);
				break;
            case 'k':
			case KEY_UP:
				menu_driver(mode_select, REQ_UP_ITEM);
				break;
            default:
                break;
        }
    } while (c != 'q' && c != 10);

    k = -1;
    if (c == 10) {
        currentItem = current_item(mode_select);
        if (currentItem != NULL) {
            k = item_index(currentItem);
        }
    }

    unpost_menu(mode_select);
    free_menu(mode_select);
    for (i=0; i<3; i++) free_item(modes[i]);
    wclear(mode_menu_win);
    wborder(mode_menu_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(mode_menu_win);
    delwin(mode_menu_win);
    wrefresh(dev_win);

    return k;
}


void g300_print_mode(WINDOW *dev_win, u_char *mode_data, int sel, bool highlight, bool modified) {
    int i, width, height, scheme_x, scheme_y, menu_x, menu_y;
    short mode, color, scheme;
    char buf[STR_LEN], *po;

    if (dev_win == NULL || mode_data == NULL) return;
    
    getmaxyx(dev_win, height, width);
    wclear(dev_win);
    box(dev_win, 0, 0);

    buf[0] = ' ';
    po = buf + 1;
    
    mode = GET_MODE_IND(mode_data[0]) + 1;
    color = mode_data[1];

    /* Header */
	mvwaddch(dev_win, 2, 0, ACS_LTEE);
	mvwhline(dev_win, 2, 1, ACS_HLINE, width-2);
	mvwaddch(dev_win, 2, width-1, ACS_RTEE);
    mvwprintw(dev_win, 1, 1, "Mode selected: %u", mode);
    if (modified) wprintw(dev_win, "*");
    print_right(dev_win, 1, 1, width-2, NAME, COLOR_PAIR(7));
	mvwprintw(dev_win, height - 2, 1, "\"q\" to exit, \"m\" to change mode, \"s\" to save mode settings, \"?\" for help");

    /* Mouse scheme */
    if (height > 30 && width > 49) {
        /* TODO Save scheme ascii on memory to avoid reading a file on each print */
        scheme = TRUE;
        scheme_y = height/2 - 12;
        scheme_x = width - 24 - scheme_y;   /* To be centered */
        for (i=0; i<24; i++) {
            mvwprintw(dev_win, scheme_y+i, scheme_x, scheme_ascii[i]);
        }
    } else {
        scheme = FALSE;
    }

    menu_y = height/2 - 10;
    menu_x = menu_y;   /* To be centered */

    /* Content: color */
    wattron(dev_win, A_UNDERLINE);
    wattron(dev_win, A_BOLD);
    mvwprintw(dev_win, menu_y, menu_x, "Colors");
    wattroff(dev_win, A_BOLD);
    wattroff(dev_win, A_UNDERLINE);
    if (sel == 0 && highlight) {
        wattron(dev_win, A_STANDOUT);
    }
    mvwprintw(dev_win, menu_y + 2, menu_x, "Color:");  /* Item #1 */
    if (sel == 0 && highlight) {
        wattroff(dev_win, A_STANDOUT);
    }
    wattron(dev_win, COLOR_PAIR(color));
    wprintw(dev_win, " %s", s_color[color]);
    if (scheme) {
        if (sel != 0 || highlight) {
            for (i=0; i<4; i++) {
                mvwprintw(dev_win, scheme_y+10+i, scheme_x, "▐");
                mvwprintw(dev_win, scheme_y+10+i, scheme_x+22, "▌");
            }
        }
    }
    wattroff(dev_win, COLOR_PAIR(color));

    /* Content: dpi */
    wattron(dev_win, A_UNDERLINE);
    wattron(dev_win, A_BOLD);
    mvwprintw(dev_win, menu_y + 5, menu_x, "DPI");
    wattroff(dev_win, A_BOLD);
    wattroff(dev_win, A_UNDERLINE);
    if (sel == 1 && highlight) {
        wattron(dev_win, A_STANDOUT);
    }
    mvwprintw(dev_win, menu_y + 7, menu_x, "Level 1:");  /* Item #2 */
    if (sel == 1 && highlight) {
        wattroff(dev_win, A_STANDOUT);
    }
    wprintw(dev_win, " %d", DPI_VAL(mode_data[3] & 0x0f));
    if (sel == 2 && highlight) {
        wattron(dev_win, A_STANDOUT);
    }
    mvwprintw(dev_win, menu_y + 8, menu_x, "Level 2:");  /* Item #3 */
    if (sel == 2 && highlight) {
        wattroff(dev_win, A_STANDOUT);
    }
    wprintw(dev_win, " %d", DPI_VAL(mode_data[4] & 0x0f));
    if (sel == 3 && highlight) {
        wattron(dev_win, A_STANDOUT);
    }
    mvwprintw(dev_win, menu_y + 9, menu_x, "Level 3:");  /* Item #4 */
    if (sel == 3 && highlight) {
        wattroff(dev_win, A_STANDOUT);
    }
    wprintw(dev_win, " %d", DPI_VAL(mode_data[5] & 0x0f));
    if (sel == 4 && highlight) {
        wattron(dev_win, A_STANDOUT);
    }
    mvwprintw(dev_win, menu_y + 10, menu_x, "Level 4:");  /* Item #5 */
    if (sel == 4 && highlight) {
        wattroff(dev_win, A_STANDOUT);
    }
    wprintw(dev_win, " %d", DPI_VAL(mode_data[6] & 0x0f));
    if (sel == 5 && highlight) {
        wattron(dev_win, A_STANDOUT);
    }
    mvwprintw(dev_win, scheme_y + 11, scheme_x + 10, "[9]");
    mvwprintw(dev_win, menu_y + 11, menu_x, "DPI Shift:");  /* Item #6 */
    if (sel == 5 && highlight) {
        wattroff(dev_win, A_STANDOUT);
    }
    wprintw(dev_win, " %s", DPI_SHIFT(mode_data[7]));

    /* Content: buttons */
    wattron(dev_win, A_UNDERLINE);
    wattron(dev_win, A_BOLD);
    mvwprintw(dev_win, menu_y + 14, menu_x, "Buttons");
    wattroff(dev_win, A_BOLD);
    wattroff(dev_win, A_UNDERLINE);
    if (sel == 6 && highlight) {
        wattron(dev_win, A_STANDOUT);
    }
    mvwprintw(dev_win, scheme_y + 7, scheme_x, "[4|");
    mvwprintw(dev_win, scheme_y + 8, scheme_x, "[_|");
    mvwprintw(dev_win, menu_y + 16, menu_x, "Button G4:");  /* Item #7 */
    if (sel == 6 && highlight) {
        wattroff(dev_win, A_STANDOUT);
    }
    g300_get_key_str(mode_data + 17, po);
    wprintw(dev_win, buf);
    if (sel == 7 && highlight) {
        wattron(dev_win, A_STANDOUT);
    }
    mvwprintw(dev_win, scheme_y + 5, scheme_x, "[5|");
    mvwprintw(dev_win, scheme_y + 6, scheme_x, "[_|");
    mvwprintw(dev_win, menu_y + 17, menu_x, "Button G5:");  /* Item #8 */
    if (sel == 7 && highlight) {
        wattroff(dev_win, A_STANDOUT);
    }
    g300_get_key_str(mode_data + 20, po);
    wprintw(dev_win, buf);
    if (sel == 8 && highlight) {
        wattron(dev_win, A_STANDOUT);
    }
    mvwprintw(dev_win, scheme_y + 7, scheme_x + 20, "|6]");
    mvwprintw(dev_win, scheme_y + 8, scheme_x + 20, "|_]");
    mvwprintw(dev_win, menu_y + 18, menu_x, "Button G6:");  /* Item #9 */
    if (sel == 8 && highlight) {
        wattroff(dev_win, A_STANDOUT);
    }
    g300_get_key_str(mode_data + 23, po);
    wprintw(dev_win, buf);
    if (sel == 9 && highlight) {
        wattron(dev_win, A_STANDOUT);
    }
    mvwprintw(dev_win, scheme_y + 5, scheme_x + 20, "|7]");
    mvwprintw(dev_win, scheme_y + 6, scheme_x + 20, "|_]");
    mvwprintw(dev_win, menu_y + 19, menu_x, "Button G7:");  /* Item #10 */
    if (sel == 9 && highlight) {
        wattroff(dev_win, A_STANDOUT);
    }
    g300_get_key_str(mode_data + 26, po);
    wprintw(dev_win, buf);
    
    wrefresh(dev_win);

    return;
}

void g300_get_key_str(u_char *but, char *str) {
    /* Adapted from ratslap */

    int m;
    unsigned char ky;

    // Modifiers (but[1])
    // 0xe0 - 0xe7 match modifiers 0x01, 0x02, 0x04 ... 0x80
        
    ky = 0xe0;
    m = 0x00;
    for (m = 0x01; m <= 0x80; m *= 2) {
        if (but[1] & m) str += sprintf(str, "%s + ", s_keys[ky]);
        ++ky;
    }

    // Buttons   (but[0])
    if (but[0] & 0x0f) {
        str += sprintf(str, "%s", s_buttons[but[0] & 0x0f]);

        if (but[2] > 0) str += sprintf(str, " + ");
    }

    // Keys      (but[2])
    if (but[2] > 0) sprintf(str, "%s", s_keys[but[2]]);

    return;
}


short g300_change_color(WINDOW *dev_win, u_char *mode_data) {
    MENU *color_select;
    ITEM *colors[9], *currentItem;
    WINDOW *color_menu_win;
    int height, width, c;
    short i, ret;

    if (dev_win == NULL) return -1;

    for (i=0; i<8; i++) {
        colors[i] = new_item(s_color[i], NULL);
    }
    colors[8] = NULL;

    color_select = new_menu(colors);
    getmaxyx(dev_win, height, width);
    color_menu_win = newwin(12,21,height/2 - 6, width/2 - 11);
    keypad(color_menu_win, TRUE);
    set_menu_win(color_select, color_menu_win);
    set_menu_sub(color_select, derwin(color_menu_win, 8, 10, 3, 4));
    set_menu_mark(color_select, " > ");
    box(color_menu_win, 0, 0);
    print_in_middle(color_menu_win, 1, 0, 21, "Select a color:", COLOR_PAIR(3));
	mvwaddch(color_menu_win, 2, 0, ACS_LTEE);
	mvwhline(color_menu_win, 2, 1, ACS_HLINE, 19);
	mvwaddch(color_menu_win, 2, 20, ACS_RTEE);
	wrefresh(dev_win);
    post_menu(color_select);
    wrefresh(color_menu_win);

    do {
        c = wgetch(color_menu_win);
        switch(c) {
            case 'j':
            case KEY_DOWN:
                currentItem = current_item(color_select);
				menu_driver(color_select, REQ_DOWN_ITEM);
				break;
            case 'k':
			case KEY_UP:
				menu_driver(color_select, REQ_UP_ITEM);
				break;
            default:
                break;
        }
    } while (c != 'q' && c != 10);

    ret = FALSE;
    if (c == 10) {
        currentItem = current_item(color_select);
        if (currentItem != NULL) {
            i = item_index(currentItem);
            mode_data[1] = i;
            ret = TRUE;
        }
    }

    unpost_menu(color_select);
    free_menu(color_select);
    for (i=0; i<8; i++) free_item(colors[i]);
    wclear(color_menu_win);
    wborder(color_menu_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(color_menu_win);
    delwin(color_menu_win);
    wrefresh(dev_win);

    return ret;
}

short g300_change_dpi(WINDOW *dev_win, u_char *mode_data, short level) {
    WINDOW *dpi_select_win;
    int height, width, c, dpi, dpi_ini;
    short lvl;

    getmaxyx(dev_win, height, width);
    dpi_select_win = newwin(7,21,height/2 - 6, width/2 - 11);
    keypad(dpi_select_win, TRUE);
    box(dpi_select_win, 0, 0);
    print_in_middle(dpi_select_win, 1, 0, 21, "Select a dpi value:", COLOR_PAIR(3));
	mvwaddch(dpi_select_win, 2, 0, ACS_LTEE);
	mvwhline(dpi_select_win, 2, 1, ACS_HLINE, 19);
	mvwaddch(dpi_select_win, 2, 20, ACS_RTEE);
    wattron(dpi_select_win, A_STANDOUT);
    wattron(dpi_select_win, A_BOLD);
    mvwprintw(dpi_select_win, 4, 2, "-");
    mvwprintw(dpi_select_win, 4, 18, "+");
    wattroff(dpi_select_win, A_BOLD);
    wattroff(dpi_select_win, A_STANDOUT);
	wrefresh(dev_win);
    wrefresh(dpi_select_win);

    lvl = level + 3;
    dpi_ini = DPI_VAL(mode_data[lvl] & 0x0f);
    dpi = dpi_ini;

    c = -1;
    do {
        switch (c) {
            case 'j':
            case '-':
            case 'h':
            case KEY_DOWN:
            case KEY_LEFT:
                if (dpi > 250) dpi -= 250;
                break;

            case 'k':
            case '+':
            case 'l':
            case KEY_UP:
            case KEY_RIGHT:
                if (dpi < 3750) dpi += 250;
                break;

            default:
                break;
        }
        wattron(dpi_select_win, A_STANDOUT);
        mvwprintw(dpi_select_win, 4, 9, "%4d", dpi);
        wattroff(dpi_select_win, A_STANDOUT);
        wrefresh(dpi_select_win);
        c = wgetch(dpi_select_win);
    } while (c != 'q' && c != 10);

    if (c == 10 && dpi != dpi_ini) {
        mode_data[lvl] = (dpi/250) & 0xf;
        return TRUE;
    }

    return FALSE;
}

void g300_toggle_dpi_shift(u_char *mode_data) {
    u_char *def, *shift;

    def = mode_data + 3;
    shift = mode_data + 7;

    if (*shift & 0x40) {
        *shift = *def & 0x0f;
    } else {
        *shift |= 0x40;
    }

    return;
}

short g300_change_button(WINDOW *dev_win, u_char *mode_data, short button) {
    return 0;
}