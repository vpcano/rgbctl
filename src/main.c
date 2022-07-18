#include <stdio.h>
#include <stdlib.h>
#include <menu.h>
#include <errno.h>
#include <locale.h>
#include <libusb-1.0/libusb.h>
#include "known_devices.h"
#include "device.h"
#include "helpers.h"

#define MENU_WIDTH 40
#define MIN(a,b) (a < b ? a : b)

int start();
int end();
libusb_context *ctx = NULL;


int main(int argc, char **argv) {
    int i, j, k, c, menu_height, menu_startx, menu_starty, n_devs, n_all_devs, flag;
    libusb_device **all_devs;
    struct libusb_device_descriptor desc;
    MENU *device_select;
    WINDOW *device_menu_win, *dev_win;
    ITEM *dev_items[KNOWN_DEVICES_LEN+1], *currentItem = NULL;
    libusb_device_handle *dev_handles[KNOWN_DEVICES_LEN];
    DeviceCallback dev_cbs[KNOWN_DEVICES_LEN];


    if (start() != OK) return EXIT_FAILURE;


    n_all_devs = libusb_get_device_list(ctx, &all_devs);
    if (n_all_devs < 0) {
        end();
        fprintf(stderr, "Error getting usb device list\n");
        return EXIT_FAILURE;
    }
    for (j=0, k=0; j<n_all_devs && k<KNOWN_DEVICES_LEN; j++) {
        if (libusb_get_device_descriptor(all_devs[j], &desc) < 0) {
            libusb_free_device_list(all_devs, 1);
            end();
            fprintf(stderr, "Error getting usb device descriptor\n");
            return EXIT_FAILURE;
        }
        for (i=0; i<KNOWN_DEVICES_LEN; i++) {
            if (desc.idVendor == known_devices[i]->vid && desc.idProduct == known_devices[i]->pid) {
                flag = libusb_open(all_devs[j], dev_handles+k);   /* Create a new reference */
                if (flag != 0) {
                    for (i=0; i<k; i++) {
                        free_item(dev_items[i]);
                        libusb_close(dev_handles[i]);
                    }
                    libusb_free_device_list(all_devs, 1);
                    end();
                    if (flag == LIBUSB_ERROR_ACCESS) {
                        fprintf(stderr, "Please, run this program as root or grant access to this USB device\n");
                    } else {
                        fprintf(stderr, "Unknown error opening USB device\n");
                    }
                    return EXIT_FAILURE;
                }
                dev_items[k] = new_item(known_devices[i]->name, TYPE_STR(known_devices[i]->type));
                dev_cbs[k] = known_devices[i]->cb;
                k++;
                break;
            }
        }
    }
    n_devs = k;
    dev_items[n_devs] = (ITEM*) NULL;
    libusb_free_device_list(all_devs, 1);  /* Close references that will never be used */

    device_select = new_menu(dev_items);
    menu_height = MIN(n_devs+4, LINES-6);
    menu_startx = (COLS/2) - (MENU_WIDTH/2);
    menu_starty = (LINES/2) - (menu_height/2);
    device_menu_win = newwin(menu_height, MENU_WIDTH, menu_starty, menu_startx);
    keypad(device_menu_win, TRUE);
    set_menu_win(device_select, device_menu_win);
    set_menu_sub(device_select, derwin(device_menu_win, n_devs, 38, 3, 1));
    set_menu_mark(device_select, " > ");
    box(device_menu_win, 0, 0);
    print_in_middle(device_menu_win, 1, 0, 40, "Select a device:", COLOR_PAIR(1));
	mvwaddch(device_menu_win, 2, 0, ACS_LTEE);
	mvwhline(device_menu_win, 2, 1, ACS_HLINE, 38);
	mvwaddch(device_menu_win, 2, 39, ACS_RTEE);
	mvprintw(LINES - 2, 1, "\"q\" to exit, \"?\" for help");
	refresh();
    post_menu(device_select);
    wrefresh(device_menu_win);

    while((c = wgetch(device_menu_win)) != 'q') {       
        switch(c) {
            case 'j':
            case KEY_DOWN:
				menu_driver(device_select, REQ_DOWN_ITEM);
				break;
            case 'k':
			case KEY_UP:
				menu_driver(device_select, REQ_UP_ITEM);
				break;
            case '?':
                // TODO: Print help
                break;
            case 10:
                currentItem = current_item(device_select);
                if (currentItem == NULL) break;
                k = item_index(currentItem);
                if (k == ERR) {
                    currentItem = NULL;
                    break;
                }
                unpost_menu(device_select);
                wrefresh(device_menu_win);
                dev_win = newwin(LINES, COLS, 0, 0);
                keypad(dev_win, TRUE);
                box(dev_win, 0, 0);
                refresh();
                wrefresh(dev_win);
                dev_cbs[k](dev_handles[k], dev_win);
                delwin(dev_win);
                clear();
                post_menu(device_select);
	            mvprintw(LINES - 2, 1, "\"q\" to exit, \"?\" for help");
                redrawwin(device_menu_win);
                refresh();
                break;
            default:
                break;
		}
        wrefresh(device_menu_win);
	}

    unpost_menu(device_select);
    free_menu(device_select);
    for (k=0; k<n_devs; k++) {
        free_item(dev_items[k]);
        libusb_close(dev_handles[k]);
    }


    end();
    return EXIT_SUCCESS;
}


int start() {
    /* Setup interface */
    setlocale(LC_ALL, "");
    if (initscr() == NULL) return ERR;
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    if (has_colors()) {
        start_color();
        init_pair(0, COLOR_WHITE, COLOR_BLACK);
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
        init_pair(4, COLOR_BLUE, COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(6, COLOR_CYAN, COLOR_BLACK);
        init_pair(7, COLOR_WHITE, COLOR_BLACK);

        init_pair(10, COLOR_WHITE, COLOR_BLACK);
        init_pair(11, COLOR_WHITE, COLOR_RED);
        init_pair(12, COLOR_WHITE, COLOR_GREEN);
        init_pair(13, COLOR_BLACK, COLOR_YELLOW);
        init_pair(14, COLOR_WHITE, COLOR_BLUE);
        init_pair(15, COLOR_BLACK, COLOR_MAGENTA);
        init_pair(16, COLOR_BLACK, COLOR_CYAN);
        init_pair(17, COLOR_BLACK, COLOR_WHITE);
    }

    /* Setup libusb */
    if (libusb_init(&ctx) < 0) {
        endwin(); /* End interface */
        return ERR;
    }

    return OK;
}

int end() {
    endwin();
    libusb_exit(ctx);
    return OK;
}