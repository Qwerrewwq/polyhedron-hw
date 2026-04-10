#ifndef TK_DRAWER_H
#define TK_DRAWER_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include "r3.h"

/* Window size */
#define SIZE 900
/* Scale factor */
#define SCALE 1.5

typedef struct {
    Display *display;
    Window window;
    GC gc;
    int screen;
} TkDrawer;

/* Initialize the drawer */
int tk_init(TkDrawer *tk);

/* Close the drawer */
void tk_close(TkDrawer *tk);

/* Clear the canvas (fill with white) */
void tk_clean(TkDrawer *tk);

/* Draw a line between two 3D points */
void tk_draw_line(TkDrawer *tk, R3 p, R3 q);

/* Transform x coordinate */
static inline int tk_x(R3 p) {
    return SIZE / 2 + (int)(SCALE * p.x);
}

/* Transform y coordinate */
static inline int tk_y(R3 p) {
    return SIZE / 2 - (int)(SCALE * p.y);
}

#endif /* TK_DRAWER_H */
