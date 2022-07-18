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

#define STR_LEN 32

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


short g300_mode_menu(WINDOW *dev_win);
short g300_load_mode(libusb_device_handle *dev_handle, uint16_t mode, u_char *mode_data);
void g300_print_mode(WINDOW *dev_win, u_char *mode_data);
void g300_cb(libusb_device_handle *dev_handle, WINDOW *dev_win);


Device Logitech_G300 = {
    NAME,
    MOUSE,
    VID,
    PID,
    g300_cb
};

void g300_cb(libusb_device_handle *dev_handle, WINDOW *dev_win) {
    int c, f;
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
    g300_print_mode(dev_win, mode_data);

    while((c = wgetch(dev_win)) != 'q') {
        switch (c) {
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
            // ...
        }
        /* Print mode */
        g300_print_mode(dev_win, mode_data);
    }

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


void g300_print_mode(WINDOW *dev_win, u_char *mode_data) {
    int i, width, height;
    short mode, color;

    if (dev_win == NULL || mode_data == NULL) return;
    
    getmaxyx(dev_win, height, width);
    wclear(dev_win);
    box(dev_win, 0, 0);
    
    char rawout[255]
         ,*po = rawout + sprintf(rawout, "RAW: ");
    for (i = 0; i < 35; ++i) {
        sprintf(po, "%.2x", (mode_data)[i]);
        po += strlen(po);
        if ((i+1) % 4 == 0) sprintf(po, " ");
        po += strlen(po);
    }
    *po = '\0';

    mode = GET_MODE_IND(mode_data[0]) + 1;
    color = mode_data[1];

    /* Header */
	mvwaddch(dev_win, 2, 0, ACS_LTEE);
	mvwhline(dev_win, 2, 1, ACS_HLINE, width-2);
	mvwaddch(dev_win, 2, width-1, ACS_RTEE);
    mvwprintw(dev_win, 1, 1, "Mode selected: %u", mode);
    print_right(dev_win, 1, 1, width-2, NAME, COLOR_PAIR(7));

    /* Content: color */
    mvwprintw(dev_win, 3, 2, "Color: ");
    attron(COLOR_PAIR(color));
    mvwprintw(dev_win, 3, 10, s_color[color]);
    attroff(COLOR_PAIR(color));

    print_in_middle(dev_win, LINES/2-1, 0, COLS, rawout, COLOR_PAIR(1));
    print_in_middle(dev_win, LINES/2, 0, COLS, "Press \"q\" to exit...", COLOR_PAIR(1));
    wrefresh(dev_win);

    return;
}