#ifndef POLYEDR_H
#define POLYEDR_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "r3.h"
#include "segment.h"

#define PI 3.14159265358979323846

/* Forward declarations */
typedef struct Edge Edge;
typedef struct Facet Facet;
typedef struct Polyedr Polyedr;

/* Edge structure */
struct Edge {
    R3 beg, fin;
    Segment *gaps;      /* Dynamic array of gaps (segments) */
    int gaps_count;     /* Number of gaps in the array */
    int gaps_capacity;  /* Allocated capacity for gaps */
};

/* Facet structure */
struct Facet {
    R3 *vertexes;       /* Array of vertices */
    int vertex_count;   /* Number of vertices */
};

/* Polyedr structure */
struct Polyedr {
    R3 *vertexes;       /* Array of all vertices */
    int vertex_count;   /* Total number of vertices */
    Edge *edges;        /* Array of all edges */
    int edge_count;     /* Total number of edges */
    Facet *facets;      /* Array of all facets */
    int facet_count;    /* Total number of facets */
    R3 V;               /* Projection vector (0, 0, 1) */
};

/* Edge constants */
#define EDGE_SBEG 0.0
#define EDGE_SFIN 1.0

/* Initialize an edge with beginning and end points */
void edge_init(Edge *e, R3 beg, R3 fin);

/* Free edge resources */
void edge_free(Edge *e);

/* Convert 1D coordinate to 3D point on the edge */
R3 edge_r3(Edge *e, double t);

/* Intersect edge with half-space defined by point a and normal n */
Segment edge_intersect_edge_with_normal(Edge *e, R3 a, R3 n);

/* Apply shadow from a facet to this edge */
void edge_shadow(Edge *e, Facet *f, R3 V);

/* Initialize a facet with an array of vertices */
void facet_init(Facet *f, R3 *vertexes, int count);

/* Free facet resources */
void facet_free(Facet *f);

/* Check if facet is vertical */
int facet_is_vertical(Facet *f, R3 V);

/* Get horizontal normal of the facet */
R3 facet_h_normal(Facet *f, R3 V);

/* Get vertical normals (returns dynamically allocated array) */
R3* facet_v_normals(Facet *f, R3 V, int *count);

/* Get center of the facet */
R3 facet_center(Facet *f);

/* Initialize polyedr from file */
int polyedr_init(Polyedr *p, const char *filename);

/* Create a test cube polyedr programmatically */
int polyedr_create_cube(Polyedr *p);

/* Free polyedr resources */
void polyedr_free(Polyedr *p);

#endif /* POLYEDR_H */
