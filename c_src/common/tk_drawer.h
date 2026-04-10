#ifndef TK_DRAWER_H
#define TK_DRAWER_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <math.h>
#include "common/r3.h"

/* Размер окна */
#define SIZE 900
/* Базовый коэффициент гомотетии */
#define BASE_SCALE 1.5

typedef struct {
    Display *display;
    Window window;
    GC gc;
    double scale;
    double offset_x;
    double offset_y;
    int auto_scale;
} TkDrawer;

/* Преобразование x-координаты */
static inline int x_coord(TkDrawer *tk, R3 p) {
    if (tk->auto_scale) {
        return SIZE / 2 + (int)(tk->scale * (p.x - tk->offset_x));
    }
    return SIZE / 2 + (int)(BASE_SCALE * p.x);
}

/* Преобразование y-координаты */
static inline int y_coord(TkDrawer *tk, R3 p) {
    if (tk->auto_scale) {
        return SIZE / 2 - (int)(tk->scale * (p.y - tk->offset_y));
    }
    return SIZE / 2 - (int)(BASE_SCALE * p.y);
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
    
    /* Инициализация параметров масштабирования */
    tk->auto_scale = 0;
    tk->scale = BASE_SCALE;
    tk->offset_x = 0.0;
    tk->offset_y = 0.0;
    
    return tk;
}

/* Настройка автоматического масштабирования под объект */
void tk_auto_scale(TkDrawer *tk, R3 *vertexes, int count) {
    if (count == 0 || !tk) return;
    
    double min_x = vertexes[0].x, max_x = vertexes[0].x;
    double min_y = vertexes[0].y, max_y = vertexes[0].y;
    
    for (int i = 1; i < count; i++) {
        if (vertexes[i].x < min_x) min_x = vertexes[i].x;
        if (vertexes[i].x > max_x) max_x = vertexes[i].x;
        if (vertexes[i].y < min_y) min_y = vertexes[i].y;
        if (vertexes[i].y > max_y) max_y = vertexes[i].y;
    }
    
    double range_x = max_x - min_x;
    double range_y = max_y - min_y;
    double max_range = (range_x > range_y) ? range_x : range_y;
    
    if (max_range > 0) {
        tk->scale = 0.9 * SIZE / max_range;
        tk->offset_x = (min_x + max_x) / 2.0;
        tk->offset_y = (min_y + max_y) / 2.0;
        tk->auto_scale = 1;
    }
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
              x_coord(tk, p), y_coord(tk, p), x_coord(tk, q), y_coord(tk, q));
    XFlush(tk->display);
}

/* Ожидание нажатия клавиши */
void tk_wait_key(TkDrawer *tk) {
    XEvent event;
    XNextEvent(tk->display, &event);
}

#endif /* TK_DRAWER_H */
