#ifndef POLYEDR_SHADOW_H
#define POLYEDR_SHADOW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "common/r3.h"
#include "common/tk_drawer.h"

/* Одномерный отрезок */
typedef struct {
    double beg;
    double fin;
} Segment;

/* Проверка на вырожденность отрезка */
static inline int segment_is_degenerate(Segment s) {
    return s.beg >= s.fin;
}

/* Пересечение с отрезком */
static inline void segment_intersect(Segment *s, Segment other) {
    if (other.beg > s->beg) {
        s->beg = other.beg;
    }
    if (other.fin < s->fin) {
        s->fin = other.fin;
    }
}

/* Разность отрезков - возвращает массив из двух отрезков */
typedef struct {
    Segment segments[2];
} SegmentPair;

static inline SegmentPair segment_subtraction(Segment s, Segment other) {
    SegmentPair result;
    
    /* Используем x,y для хранения beg,fin так как Segment имеет beg,fin */
    result.segments[0].beg = s.beg;
    result.segments[0].fin = (s.fin < other.beg) ? s.fin : other.beg;
    result.segments[1].beg = (s.beg > other.fin) ? s.beg : other.fin;
    result.segments[1].fin = s.fin;
    
    return result;
}

/* Ребро полиэдра */
typedef struct EdgeStruct {
    R3 beg;
    R3 fin;
    /* Список «просветов» */
    Segment gaps[100];
    int gap_count;
} Edge;

#define EDGE_SBEG 0.0
#define EDGE_SFIN 1.0

/* Грань полиэдра */
#define MAX_VERTEXES_PER_FACET 100
typedef struct FacetStruct {
    R3 vertexes[MAX_VERTEXES_PER_FACET];
    int count;
} Facet;

/* Полиэдр */
#define MAX_VERTEXES 5000
#define MAX_EDGES 20000
#define MAX_FACETS 5000

/* Вектор проектирования */
static const R3 POLYEDR_V = {0.0, 0.0, 1.0};

typedef struct {
    R3 vertexes[MAX_VERTEXES];
    int vertex_count;
    Edge edges[MAX_EDGES];
    int edge_count;
    Facet facets[MAX_FACETS];
    int facet_count;
} Polyedr;

/* Создание инициализированного ребра */
static inline Edge edge_create(R3 beg, R3 fin) {
    Edge e;
    e.beg = beg;
    e.fin = fin;
    e.gaps[0].beg = EDGE_SBEG;
    e.gaps[0].fin = EDGE_SFIN;
    e.gap_count = 1;
    return e;
}

/* Преобразование одномерных координат в трёхмерные */
static inline R3 edge_r3(Edge *e, double t) {
    return r3_add(r3_mul(e->beg, EDGE_SFIN - t), r3_mul(e->fin, t));
}

/* Пересечение ребра с полупространством */
static inline Segment edge_intersect_edge_with_normal(Edge *e, R3 a, R3 n) {
    double f0 = r3_dot(r3_sub(e->beg, a), n);
    double f1 = r3_dot(r3_sub(e->fin, a), n);
    
    if (f0 >= 0.0 && f1 >= 0.0) {
        return (Segment){EDGE_SFIN, EDGE_SBEG};
    }
    if (f0 < 0.0 && f1 < 0.0) {
        return (Segment){EDGE_SBEG, EDGE_SFIN};
    }
    
    double x = -f0 / (f1 - f0);
    if (f0 < 0.0) {
        return (Segment){EDGE_SBEG, x};
    } else {
        return (Segment){x, EDGE_SFIN};
    }
}

/* «Вертикальна» ли грань? */
static inline int facet_is_vertical(Facet *f) {
    R3 n = r3_cross(r3_sub(f->vertexes[1], f->vertexes[0]), 
                    r3_sub(f->vertexes[2], f->vertexes[0]));
    return r3_dot(n, POLYEDR_V) == 0.0;
}

/* Нормаль к «горизонтальному» полупространству */
static inline R3 facet_h_normal(Facet *f) {
    R3 n = r3_cross(r3_sub(f->vertexes[1], f->vertexes[0]), 
                    r3_sub(f->vertexes[2], f->vertexes[0]));
    if (r3_dot(n, POLYEDR_V) < 0.0) {
        return r3_mul(n, -1.0);
    }
    return n;
}

/* Центр грани */
static inline R3 facet_center(Facet *f) {
    R3 sum = {0.0, 0.0, 0.0};
    for (int i = 0; i < f->count; i++) {
        sum = r3_add(sum, f->vertexes[i]);
    }
    return r3_mul(sum, 1.0 / f->count);
}

/* Нормали к «вертикальным» полупространствам */
static inline R3 facet_v_normal_at(Facet *f, int k) {
    R3 n = r3_cross(r3_sub(f->vertexes[k], f->vertexes[(k - 1 + f->count) % f->count]), POLYEDR_V);
    R3 center = facet_center(f);
    if (r3_dot(n, r3_sub(f->vertexes[(k - 1 + f->count) % f->count], center)) < 0.0) {
        return r3_mul(n, -1.0);
    }
    return n;
}

/* Учёт тени от одной грани */
static inline void edge_shadow(Edge *e, Facet *f) {
    /* «Вертикальная» грань не затеняет ничего */
    R3 n0 = r3_cross(r3_sub(f->vertexes[1], f->vertexes[0]), 
                     r3_sub(f->vertexes[2], f->vertexes[0]));
    if (r3_dot(n0, POLYEDR_V) == 0.0) {
        return;
    }
    
    /* Нахождение одномерной тени на ребре */
    Segment shade = {EDGE_SBEG, EDGE_SFIN};
    
    for (int i = 0; i < f->count; i++) {
        int next_i = (i + 1) % f->count;
        R3 u = f->vertexes[i];
        R3 v = facet_v_normal_at(f, next_i);
        Segment s = edge_intersect_edge_with_normal(e, u, v);
        segment_intersect(&shade, s);
        if (segment_is_degenerate(shade)) {
            return;
        }
    }
    
    Segment s = edge_intersect_edge_with_normal(e, f->vertexes[0], facet_h_normal(f));
    segment_intersect(&shade, s);
    if (segment_is_degenerate(shade)) {
        return;
    }
    
    /* Преобразование списка «просветов», если тень невырождена */
    Segment new_gaps[200];
    int new_gap_count = 0;
    
    for (int g = 0; g < e->gap_count; g++) {
        SegmentPair pair = segment_subtraction(e->gaps[g], shade);
        if (!segment_is_degenerate(pair.segments[0])) {
            new_gaps[new_gap_count++] = pair.segments[0];
        }
        if (!segment_is_degenerate(pair.segments[1])) {
            new_gaps[new_gap_count++] = pair.segments[1];
        }
    }
    
    e->gap_count = new_gap_count;
    for (int i = 0; i < new_gap_count; i++) {
        e->gaps[i] = new_gaps[i];
    }
}

/* Инициализация полиэдра из файла */
Polyedr* polyedr_create(const char *filename) {
    Polyedr *p = (Polyedr*)malloc(sizeof(Polyedr));
    if (!p) return NULL;
    
    p->vertex_count = 0;
    p->edge_count = 0;
    p->facet_count = 0;
    
    FILE *f = fopen(filename, "r");
    if (!f) {
        free(p);
        return NULL;
    }
    
    char line[1024];
    int line_num = 0;
    double c, alpha, beta, gamma;
    int nv, nf, ne;
    
    while (fgets(line, sizeof(line), f)) {
        if (line_num == 0) {
            /* Первая строка: коэффициент гомотетии и углы Эйлера */
            double values[4];
            int count = 0;
            char *token = strtok(line, " \t\n");
            while (token && count < 4) {
                values[count++] = atof(token);
                token = strtok(NULL, " \t\n");
            }
            c = values[0];
            alpha = values[1] * M_PI / 180.0;
            beta = values[2] * M_PI / 180.0;
            gamma = values[3] * M_PI / 180.0;
        } else if (line_num == 1) {
            /* Вторая строка: число вершин, граней и рёбер */
            sscanf(line, "%d %d %d", &nv, &nf, &ne);
        } else if (line_num < nv + 2) {
            /* Задание всех вершин полиэдра */
            double x, y, z;
            sscanf(line, "%lf %lf %lf", &x, &y, &z);
            R3 v = r3_create(x, y, z);
            v = r3_rz(v, alpha);
            v = r3_ry(v, beta);
            v = r3_rz(v, gamma);
            v = r3_mul(v, c);
            p->vertexes[p->vertex_count++] = v;
        } else {
            /* Задание граней */
            int buf[MAX_VERTEXES_PER_FACET];
            int size = 0;
            char *token = strtok(line, " \t\n");
            while (token) {
                buf[size++] = atoi(token);
                token = strtok(NULL, " \t\n");
            }
            
            if (size > 0) {
                int vertex_count = buf[0];
                
                /* Массив вершин этой грани */
                R3 facet_vertexes[MAX_VERTEXES_PER_FACET];
                for (int i = 0; i < vertex_count; i++) {
                    facet_vertexes[i] = p->vertexes[buf[i + 1] - 1];
                }
                
                /* Задание рёбер грани */
                for (int n = 0; n < vertex_count; n++) {
                    p->edges[p->edge_count] = edge_create(
                        facet_vertexes[(n - 1 + vertex_count) % vertex_count],
                        facet_vertexes[n]
                    );
                    p->edge_count++;
                }
                
                /* Задание самой грани */
                Facet *facet = &p->facets[p->facet_count];
                facet->count = vertex_count;
                for (int i = 0; i < vertex_count; i++) {
                    facet->vertexes[i] = facet_vertexes[i];
                }
                p->facet_count++;
            }
        }
        line_num++;
    }
    
    fclose(f);
    return p;
}

/* Освобождение памяти полиэдра */
void polyedr_free(Polyedr *p) {
    if (p) free(p);
}

/* Метод изображения полиэдра */
void polyedr_draw(Polyedr *p, TkDrawer *tk) {
    tk_clean(tk);
    
    /* Настройка автоматического масштабирования */
    tk_auto_scale(tk, p->vertexes, p->vertex_count);
    
    for (int i = 0; i < p->edge_count; i++) {
        for (int j = 0; j < p->facet_count; j++) {
            edge_shadow(&p->edges[i], &p->facets[j]);
        }
        
        for (int g = 0; g < p->edges[i].gap_count; g++) {
            R3 p1 = edge_r3(&p->edges[i], p->edges[i].gaps[g].beg);
            R3 p2 = edge_r3(&p->edges[i], p->edges[i].gaps[g].fin);
            tk_draw_line(tk, p1, p2);
        }
    }
}

#endif /* POLYEDR_SHADOW_H */
