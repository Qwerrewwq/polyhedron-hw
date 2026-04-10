#include "tk_drawer.h"

int tk_init(TkDrawer *tk) {
    tk->display = XOpenDisplay(NULL);
    if (!tk->display) {
        fprintf(stderr, "Error: Cannot open display\n");
        return -1;
    }
    
    tk->screen = DefaultScreen(tk->display);
    
    /* Create window */
    tk->window = XCreateSimpleWindow(
        tk->display,
        RootWindow(tk->display, tk->screen),
        0, 0,
        SIZE + 5, SIZE + 5,
        1,
        BlackPixel(tk->display, tk->screen),
        WhitePixel(tk->display, tk->screen)
    );
    
    XStoreName(tk->display, tk->window, "Polyedr Projection");
    XSelectInput(tk->display, tk->window, ExposureMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(tk->display, tk->window);
    
    /* Wait for window to be mapped */
    XEvent e;
    while (1) {
        XNextEvent(tk->display, &e);
        if (e.type == MapNotify) {
            break;
        }
    }
    
    /* Create graphics context */
    tk->gc = XCreateGC(tk->display, tk->window, 0, NULL);
    XSetForeground(tk->display, tk->gc, BlackPixel(tk->display, tk->screen));
    XSetLineAttributes(tk->display, tk->gc, 1, LineSolid, CapButt, JoinMiter);
    
    return 0;
}

void tk_close(TkDrawer *tk) {
    if (tk->gc) {
        XFreeGC(tk->display, tk->gc);
    }
    if (tk->window) {
        XDestroyWindow(tk->display, tk->window);
    }
    if (tk->display) {
        XCloseDisplay(tk->display);
    }
}

void tk_clean(TkDrawer *tk) {
    /* Fill with white */
    XSetForeground(tk->display, tk->gc, WhitePixel(tk->display, tk->screen));
    XFillRectangle(tk->display, tk->window, tk->gc, 0, 0, SIZE, SIZE);
    XSetForeground(tk->display, tk->gc, BlackPixel(tk->display, tk->screen));
    XFlush(tk->display);
}

void tk_draw_line(TkDrawer *tk, R3 p, R3 q) {
    int x1 = tk_x(p);
    int y1 = tk_y(p);
    int x2 = tk_x(q);
    int y2 = tk_y(q);
    
    XDrawLine(tk->display, tk->window, tk->gc, x1, y1, x2, y2);
    XFlush(tk->display);
}
