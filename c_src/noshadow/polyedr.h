#ifndef POLYEDR_NOSHADOW_H
#define POLYEDR_NOSHADOW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "common/r3.h"
#include "common/tk_drawer.h"

/* Ребро полиэдра */
typedef struct {
    R3 beg;
    R3 fin;
} Edge;

/* Грань полиэдра */
#define MAX_VERTEXES_PER_FACET 100
typedef struct {
    R3 vertexes[MAX_VERTEXES_PER_FACET];
    int count;
} Facet;

/* Полиэдр */
#define MAX_VERTEXES 5000
#define MAX_EDGES 20000
#define MAX_FACETS 5000

typedef struct {
    R3 vertexes[MAX_VERTEXES];
    int vertex_count;
    Edge edges[MAX_EDGES];
    int edge_count;
    Facet facets[MAX_FACETS];
    int facet_count;
} Polyedr;

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
                    p->edges[p->edge_count].beg = facet_vertexes[(n - 1 + vertex_count) % vertex_count];
                    p->edges[p->edge_count].fin = facet_vertexes[n];
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
        tk_draw_line(tk, p->edges[i].beg, p->edges[i].fin);
    }
}

#endif /* POLYEDR_NOSHADOW_H */
