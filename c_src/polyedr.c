#include "polyedr.h"

/* Initialize an edge with beginning and end points */
void edge_init(Edge *e, R3 beg, R3 fin) {
    e->beg = beg;
    e->fin = fin;
    e->gaps_count = 1;
    e->gaps_capacity = 8;
    e->gaps = (Segment*)malloc(e->gaps_capacity * sizeof(Segment));
    if (e->gaps) {
        e->gaps[0] = segment_create(EDGE_SBEG, EDGE_SFIN);
    }
}

/* Free edge resources */
void edge_free(Edge *e) {
    if (e->gaps) {
        free(e->gaps);
        e->gaps = NULL;
    }
}

/* Convert 1D coordinate to 3D point on the edge */
R3 edge_r3(Edge *e, double t) {
    /* return e->beg * (EDGE_SFIN - t) + e->fin * t */
    R3 term1 = r3_mul(e->beg, EDGE_SFIN - t);
    R3 term2 = r3_mul(e->fin, t);
    return r3_add(term1, term2);
}

/* Intersect edge with half-space defined by point a and normal n */
Segment edge_intersect_edge_with_normal(Edge *e, R3 a, R3 n) {
    R3 diff_beg = r3_sub(e->beg, a);
    R3 diff_fin = r3_sub(e->fin, a);
    double f0 = r3_dot(n, diff_beg);
    double f1 = r3_dot(n, diff_fin);
    
    if (f0 >= 0.0 && f1 >= 0.0) {
        /* Entire edge is outside - return degenerate segment [SFIN, SBEG] */
        return segment_create(EDGE_SFIN, EDGE_SBEG);
    }
    if (f0 < 0.0 && f1 < 0.0) {
        /* Entire edge is inside - return full segment [SBEG, SFIN] */
        return segment_create(EDGE_SBEG, EDGE_SFIN);
    }
    
    /* Edge crosses the plane */
    double x = -f0 / (f1 - f0);
    if (f0 < 0.0) {
        return segment_create(EDGE_SBEG, x);
    } else {
        return segment_create(x, EDGE_SFIN);
    }
}

/* Apply shadow from a facet to this edge */
void edge_shadow(Edge *e, Facet *f, R3 V) {
    /* Vertical facets don't cast shadows */
    if (facet_is_vertical(f, V)) {
        return;
    }
    
    /* Find 1D shadow on the edge */
    Segment shade = segment_create(EDGE_SBEG, EDGE_SFIN);
    
    /* Get vertical normals */
    int v_normals_count;
    R3 *v_normals = facet_v_normals(f, V, &v_normals_count);
    if (!v_normals) {
        return;
    }
    
    /* Intersect with each vertex/vertical normal pair */
    for (int i = 0; i < f->vertex_count; i++) {
        Segment intersection = edge_intersect_edge_with_normal(e, f->vertexes[i], v_normals[i]);
        segment_intersect(&shade, intersection);
        if (segment_is_degenerate(shade)) {
            free(v_normals);
            return;
        }
    }
    
    /* Intersect with horizontal normal */
    R3 h_norm = facet_h_normal(f, V);
    Segment intersection = edge_intersect_edge_with_normal(e, f->vertexes[0], h_norm);
    segment_intersect(&shade, intersection);
    
    free(v_normals);
    
    if (segment_is_degenerate(shade)) {
        return;
    }
    
    /* Transform gaps list: subtract shade from each gap */
    /* Estimate new capacity needed */
    int new_capacity = e->gaps_count * 2;
    Segment *new_gaps = (Segment*)malloc(new_capacity * sizeof(Segment));
    if (!new_gaps) {
        return;
    }
    
    int new_count = 0;
    for (int i = 0; i < e->gaps_count; i++) {
        Segment sub_result[2];
        segment_subtraction(e->gaps[i], shade, sub_result);
        
        for (int j = 0; j < 2; j++) {
            if (!segment_is_degenerate(sub_result[j])) {
                if (new_count >= new_capacity) {
                    new_capacity *= 2;
                    Segment *temp = (Segment*)realloc(new_gaps, new_capacity * sizeof(Segment));
                    if (!temp) {
                        free(new_gaps);
                        return;
                    }
                    new_gaps = temp;
                }
                new_gaps[new_count++] = sub_result[j];
            }
        }
    }
    
    /* Replace old gaps with new gaps */
    free(e->gaps);
    e->gaps = new_gaps;
    e->gaps_count = new_count;
    e->gaps_capacity = new_capacity;
}

/* Initialize a facet with an array of vertices */
void facet_init(Facet *f, R3 *vertexes, int count) {
    f->vertex_count = count;
    f->vertexes = (R3*)malloc(count * sizeof(R3));
    if (f->vertexes) {
        memcpy(f->vertexes, vertexes, count * sizeof(R3));
    }
}

/* Free facet resources */
void facet_free(Facet *f) {
    if (f->vertexes) {
        free(f->vertexes);
        f->vertexes = NULL;
    }
}

/* Check if facet is vertical */
int facet_is_vertical(Facet *f, R3 V) {
    R3 n = facet_h_normal(f, V);
    return fabs(r3_dot(n, V)) < 1e-10;
}

/* Get horizontal normal of the facet */
R3 facet_h_normal(Facet *f, R3 V) {
    /* n = (vertexes[1] - vertexes[0]) cross (vertexes[2] - vertexes[0]) */
    R3 v1 = r3_sub(f->vertexes[1], f->vertexes[0]);
    R3 v2 = r3_sub(f->vertexes[2], f->vertexes[0]);
    R3 n = r3_cross(v1, v2);
    
    /* Flip if pointing opposite to V */
    if (r3_dot(n, V) < 0.0) {
        n = r3_mul(n, -1.0);
    }
    return n;
}

/* Get vertical normals */
R3* facet_v_normals(Facet *f, R3 V, int *count) {
    *count = f->vertex_count;
    R3 *normals = (R3*)malloc(f->vertex_count * sizeof(R3));
    if (!normals) {
        return NULL;
    }
    
    R3 center = facet_center(f);
    
    for (int k = 0; k < f->vertex_count; k++) {
        int prev = (k == 0) ? f->vertex_count - 1 : k - 1;
        /* n = (vertexes[k] - vertexes[prev]) cross V */
        R3 edge_vec = r3_sub(f->vertexes[k], f->vertexes[prev]);
        R3 n = r3_cross(edge_vec, V);
        
        /* Flip if pointing inward */
        R3 to_prev = r3_sub(f->vertexes[prev], center);
        if (r3_dot(n, to_prev) < 0.0) {
            n = r3_mul(n, -1.0);
        }
        normals[k] = n;
    }
    
    return normals;
}

/* Get center of the facet */
R3 facet_center(Facet *f) {
    R3 sum = r3_create(0.0, 0.0, 0.0);
    for (int i = 0; i < f->vertex_count; i++) {
        sum = r3_add(sum, f->vertexes[i]);
    }
    double inv_count = 1.0 / f->vertex_count;
    return r3_mul(sum, inv_count);
}

/* Initialize polyedr from file */
int polyedr_init(Polyedr *p, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return -1;
    }
    
    p->V = r3_create(0.0, 0.0, 1.0);
    p->vertexes = NULL;
    p->edges = NULL;
    p->facets = NULL;
    p->vertex_count = 0;
    p->edge_count = 0;
    p->facet_count = 0;
    
    char line[1024];
    int line_num = 0;
    double c = 1.0;
    double alpha = 0.0, beta = 0.0, gamma = 0.0;
    int nv = 0, nf = 0, ne = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        if (line_num == 1) {
            /* First line: scale factor and Euler angles */
            char *token = strtok(line, " \t\n");
            if (token) {
                c = atof(token);
            }
            token = strtok(NULL, " \t\n");
            if (token) {
                alpha = atof(token) * PI / 180.0;
            }
            token = strtok(NULL, " \t\n");
            if (token) {
                beta = atof(token) * PI / 180.0;
            }
            token = strtok(NULL, " \t\n");
            if (token) {
                gamma = atof(token) * PI / 180.0;
            }
        } else if (line_num == 2) {
            /* Second line: number of vertices, facets, edges */
            sscanf(line, "%d %d %d", &nv, &nf, &ne);
            
            /* Allocate arrays */
            p->vertex_count = nv;
            p->vertexes = (R3*)malloc(nv * sizeof(R3));
            p->facet_count = nf;
            p->facets = (Facet*)malloc(nf * sizeof(Facet));
            /* Edges will be created as we process facets */
            p->edges = NULL;
            p->edge_count = 0;
            
            if (!p->vertexes || !p->facets) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                fclose(file);
                return -1;
            }
        } else if (line_num <= nv + 1) {
            /* Vertex definitions */
            double x, y, z;
            sscanf(line, "%lf %lf %lf", &x, &y, &z);
            R3 v = r3_create(x, y, z);
            v = r3_rz(v, alpha);
            v = r3_ry(v, beta);
            v = r3_rz(v, gamma);
            v = r3_mul(v, c);
            p->vertexes[line_num - 2] = v;
        } else {
            /* Facet definitions */
            char *tokens[64];
            int token_count = 0;
            char *token = strtok(line, " \t\n");
            while (token && token_count < 64) {
                tokens[token_count++] = token;
                token = strtok(NULL, " \t\n");
            }
            
            if (token_count < 1) continue;
            
            int size = atoi(tokens[0]);
            R3 *facet_vertices = (R3*)malloc(size * sizeof(R3));
            if (!facet_vertices) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                fclose(file);
                return -1;
            }
            
            for (int i = 0; i < size; i++) {
                int idx = atoi(tokens[i + 1]) - 1;  /* 1-based to 0-based */
                facet_vertices[i] = p->vertexes[idx];
            }
            
            /* Create edges for this facet */
            for (int n = 0; n < size; n++) {
                Edge new_edge;
                edge_init(&new_edge, facet_vertices[(n - 1 + size) % size], facet_vertices[n]);
                
                /* Reallocate edges array */
                p->edge_count++;
                Edge *temp = (Edge*)realloc(p->edges, p->edge_count * sizeof(Edge));
                if (!temp) {
                    fprintf(stderr, "Error: Memory allocation failed\n");
                    free(facet_vertices);
                    fclose(file);
                    return -1;
                }
                p->edges = temp;
                p->edges[p->edge_count - 1] = new_edge;
            }
            
            /* Create facet */
            facet_init(&p->facets[p->facet_count++], facet_vertices, size);
            free(facet_vertices);
        }
    }
    
    fclose(file);
    return 0;
}

/* Free polyedr resources */
void polyedr_free(Polyedr *p) {
    if (p->vertexes) {
        free(p->vertexes);
        p->vertexes = NULL;
    }
    
    if (p->edges) {
        for (int i = 0; i < p->edge_count; i++) {
            edge_free(&p->edges[i]);
        }
        free(p->edges);
        p->edges = NULL;
    }
    
    if (p->facets) {
        for (int i = 0; i < p->facet_count; i++) {
            facet_free(&p->facets[i]);
        }
        free(p->facets);
        p->facets = NULL;
    }
    
    p->vertex_count = 0;
    p->edge_count = 0;
    p->facet_count = 0;
}

/* Test utility: create a simple cube polyedr programmatically */
int polyedr_create_cube(Polyedr *p) {
    p->V = r3_create(0.0, 0.0, 1.0);
    p->vertex_count = 8;
    p->facet_count = 6;
    p->edge_count = 0;
    p->edges = NULL;
    
    /* Allocate vertices */
    p->vertexes = (R3*)malloc(8 * sizeof(R3));
    if (!p->vertexes) return -1;
    
    /* Cube vertices */
    p->vertexes[0] = r3_create(-0.5, -0.5, 0.5);
    p->vertexes[1] = r3_create(-0.5, 0.5, 0.5);
    p->vertexes[2] = r3_create(0.5, 0.5, 0.5);
    p->vertexes[3] = r3_create(0.5, -0.5, 0.5);
    p->vertexes[4] = r3_create(-0.5, -0.5, -0.5);
    p->vertexes[5] = r3_create(-0.5, 0.5, -0.5);
    p->vertexes[6] = r3_create(0.5, 0.5, -0.5);
    p->vertexes[7] = r3_create(0.5, -0.5, -0.5);
    
    /* Allocate facets */
    p->facets = (Facet*)malloc(6 * sizeof(Facet));
    if (!p->facets) {
        free(p->vertexes);
        return -1;
    }
    
    /* Initialize all facet vertex pointers to NULL */
    for (int i = 0; i < 6; i++) {
        p->facets[i].vertexes = NULL;
        p->facets[i].vertex_count = 0;
    }
    
    /* Create facets */
    R3 f1_verts[4] = {p->vertexes[0], p->vertexes[1], p->vertexes[2], p->vertexes[3]};
    facet_init(&p->facets[0], f1_verts, 4);
    
    R3 f2_verts[4] = {p->vertexes[4], p->vertexes[5], p->vertexes[1], p->vertexes[0]};
    facet_init(&p->facets[1], f2_verts, 4);
    
    R3 f3_verts[4] = {p->vertexes[2], p->vertexes[1], p->vertexes[5], p->vertexes[6]};
    facet_init(&p->facets[2], f3_verts, 4);
    
    R3 f4_verts[4] = {p->vertexes[2], p->vertexes[6], p->vertexes[7], p->vertexes[3]};
    facet_init(&p->facets[3], f4_verts, 4);
    
    R3 f5_verts[4] = {p->vertexes[0], p->vertexes[3], p->vertexes[7], p->vertexes[4]};
    facet_init(&p->facets[4], f5_verts, 4);
    
    R3 f6_verts[4] = {p->vertexes[7], p->vertexes[6], p->vertexes[5], p->vertexes[4]};
    facet_init(&p->facets[5], f6_verts, 4);
    
    /* Create edges for each facet */
    for (int f = 0; f < 6; f++) {
        for (int n = 0; n < 4; n++) {
            Edge e;
            edge_init(&e, p->facets[f].vertexes[(n - 1 + 4) % 4], p->facets[f].vertexes[n]);
            
            p->edge_count++;
            Edge *temp = (Edge*)realloc(p->edges, p->edge_count * sizeof(Edge));
            if (!temp) {
                polyedr_free(p);
                return -1;
            }
            p->edges = temp;
            p->edges[p->edge_count - 1] = e;
        }
    }
    
    return 0;
}
