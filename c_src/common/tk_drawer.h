#ifndef TK_DRAWER_H
#define TK_DRAWER_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include "common/r3.h"

/* Размер окна */
#define SIZE 900
/* Коэффициент гомотетии */
#define SCALE 1.5

typedef struct {
    Display *display;
    Window window;
    GC gc;
} TkDrawer;

/* Преобразование x-координаты */
static inline int x_coord(R3 p) {
    return SIZE / 2 + (int)(SCALE * p.x);
}

/* Преобразование y-координаты */
static inline int y_coord(R3 p) {
    return SIZE / 2 - (int)(SCALE * p.y);
}

/* Инициализация графического интерфейса */
TkDrawer* tk_create(void) {
    TkDrawer *tk = (TkDrawer*)malloc(sizeof(TkDrawer));
    if (!tk) return NULL;
    
    tk->display = XOpenDisplay(NULL);
    if (!tk->display) {
        free(tk);
        return NULL;
    }
    
    int screen = DefaultScreen(tk->display);
    tk->window = XCreateSimpleWindow(tk->display, RootWindow(tk->display, screen),
                                     0, 0, SIZE + 5, SIZE + 5, 1,
                                     BlackPixel(tk->display, screen),
                                     WhitePixel(tk->display, screen));
    
    XStoreName(tk->display, tk->window, "Изображение проекции полиэдра");
    XSelectInput(tk->display, tk->window, KeyPressMask | StructureNotifyMask);
    XMapWindow(tk->display, tk->window);
    
    tk->gc = XCreateGC(tk->display, tk->window, 0, NULL);
    XSetForeground(tk->display, tk->gc, BlackPixel(tk->display, screen));
    XSetLineAttributes(tk->display, tk->gc, 1, LineSolid, CapButt, JoinMiter);
    
    /* Обработка Ctrl+C через закрытие окна */
    Atom wm_delete = XInternAtom(tk->display, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(tk->display, tk->window, &wm_delete, 1);
    
    return tk;
}

/* Завершение работы */
void tk_close(TkDrawer *tk) {
    if (tk) {
        if (tk->display) {
            XDestroyWindow(tk->display, tk->window);
            XCloseDisplay(tk->display);
        }
        free(tk);
    }
}

/* Стирание существующей картинки */
void tk_clean(TkDrawer *tk) {
    XSetForeground(tk->display, tk->gc, WhitePixel(tk->display, DefaultScreen(tk->display)));
    XFillRectangle(tk->display, tk->window, tk->gc, 0, 0, SIZE, SIZE);
    XFlush(tk->display);
    XSetForeground(tk->display, tk->gc, BlackPixel(tk->display, DefaultScreen(tk->display)));
}

/* Рисование линии */
void tk_draw_line(TkDrawer *tk, R3 p, R3 q) {
    XDrawLine(tk->display, tk->window, tk->gc, 
              x_coord(p), y_coord(p), x_coord(q), y_coord(q));
    XFlush(tk->display);
}

/* Ожидание нажатия клавиши */
void tk_wait_key(TkDrawer *tk) {
    XEvent event;
    XNextEvent(tk->display, &event);
}

#endif /* TK_DRAWER_H */
