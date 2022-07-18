#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <curses.h>

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
void print_left(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
void print_right(WINDOW *win, int starty, int startx, int width, char *string, chtype color);

#endif