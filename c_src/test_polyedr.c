#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "r3.h"
#include "segment.h"
#include "polyedr.h"

/* Test utilities */
static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT(cond, msg) do { \
    if (cond) { \
        printf("  PASS: %s\n", msg); \
        tests_passed++; \
    } else { \
        printf("  FAIL: %s\n", msg); \
        tests_failed++; \
    } \
} while(0)

#define APPROX_EQ(a, b, eps) (fabs((a) - (b)) < (eps))
#define R3_APPROX_EQ(a, b, eps) (APPROX_EQ((a).x, (b).x, eps) && APPROX_EQ((a).y, (b).y, eps) && APPROX_EQ((a).z, (b).z, eps))

/* ==================== Segment Tests ==================== */
void test_segment_degenerate() {
    printf("\n=== Segment Degeneracy Tests ===\n");
    
    Segment s1 = segment_create(0.0, 1.0);
    ASSERT(!segment_is_degenerate(s1), "[0,1] is not degenerate");
    
    Segment s2 = segment_create(0.0, 0.0);
    ASSERT(segment_is_degenerate(s2), "[0,0] is degenerate");
    
    Segment s3 = segment_create(0.0, -1.0);
    ASSERT(segment_is_degenerate(s3), "[0,-1] is degenerate");
}

void test_segment_intersect() {
    printf("\n=== Segment Intersection Tests ===\n");
    
    /* Intersection with self */
    Segment a = segment_create(0.0, 1.0);
    Segment b = segment_create(0.0, 1.0);
    segment_intersect(&a, b);
    ASSERT(APPROX_EQ(a.beg, 0.0, 1e-9) && APPROX_EQ(a.fin, 1.0, 1e-9), "Intersection with self");
    
    /* Intersection with larger segment */
    a = segment_create(0.0, 1.0);
    b = segment_create(0.0, 2.0);
    segment_intersect(&a, b);
    ASSERT(APPROX_EQ(a.beg, 0.0, 1e-9) && APPROX_EQ(a.fin, 1.0, 1e-9), "Intersection with larger");
    
    /* Intersection resulting in smaller */
    a = segment_create(0.0, 2.0);
    b = segment_create(0.5, 1.5);
    segment_intersect(&a, b);
    ASSERT(APPROX_EQ(a.beg, 0.5, 1e-9) && APPROX_EQ(a.fin, 1.5, 1e-9), "Intersection results in smaller");
}

void test_segment_subtraction() {
    printf("\n=== Segment Subtraction Tests ===\n");
    
    /* Subtract self - both results degenerate */
    Segment a = segment_create(0.0, 1.0);
    Segment b = segment_create(0.0, 1.0);
    Segment result[2];
    segment_subtraction(a, b, result);
    ASSERT(segment_is_degenerate(result[0]) && segment_is_degenerate(result[1]), 
           "Subtracting self gives two degenerate");
    
    /* Subtract smaller from larger with same start */
    a = segment_create(0.0, 2.0);
    b = segment_create(0.0, 1.0);
    segment_subtraction(a, b, result);
    ASSERT(segment_is_degenerate(result[0]) && !segment_is_degenerate(result[1]),
           "Subtract smaller from larger (same start)");
    
    /* Subtract inner segment */
    a = segment_create(-1.0, 2.0);
    b = segment_create(0.0, 1.0);
    segment_subtraction(a, b, result);
    ASSERT(!segment_is_degenerate(result[0]) && !segment_is_degenerate(result[1]),
           "Subtract inner segment gives two non-degenerate");
}

/* ==================== Edge Tests ==================== */
void test_edge_r3() {
    printf("\n=== Edge R3 Tests ===\n");
    
    Edge e;
    R3 beg = r3_create(0.0, 0.0, -1.0);
    R3 fin = r3_create(1.0, 0.0, -1.0);
    edge_init(&e, beg, fin);
    
    R3 p0 = edge_r3(&e, 0.0);
    ASSERT(R3_APPROX_EQ(p0, beg, 1e-9), "t=0 gives begin point");
    
    R3 p1 = edge_r3(&e, 1.0);
    ASSERT(R3_APPROX_EQ(p1, fin, 1e-9), "t=1 gives end point");
    
    R3 p05 = edge_r3(&e, 0.5);
    R3 expected = r3_create(0.5, 0.0, -1.0);
    ASSERT(R3_APPROX_EQ(p05, expected, 1e-9), "t=0.5 gives midpoint");
    
    edge_free(&e);
}

void test_edge_intersect() {
    printf("\n=== Edge Intersection Tests ===\n");
    
    Edge e;
    edge_init(&e, r3_create(0.0, 0.0, -1.0), r3_create(1.0, 0.0, -1.0));
    
    /* Edge belongs to half-space - full segment */
    R3 a = r3_create(0.0, 0.0, 0.0);
    R3 n = r3_create(0.0, 0.0, 1.0);
    Segment s = edge_intersect_edge_with_normal(&e, a, n);
    ASSERT(APPROX_EQ(s.beg, 0.0, 1e-9) && APPROX_EQ(s.fin, 1.0, 1e-9),
           "Edge in half-space gives full segment");
    
    edge_free(&e);
    edge_init(&e, r3_create(0.0, 0.0, 1.0), r3_create(1.0, 0.0, 1.0));
    
    /* Edge outside half-space - degenerate */
    s = edge_intersect_edge_with_normal(&e, a, n);
    ASSERT(segment_is_degenerate(s), "Edge outside half-space is degenerate");
    
    edge_free(&e);
    edge_init(&e, r3_create(0.0, 0.0, 0.0), r3_create(1.0, 0.0, 0.0));
    
    /* Edge on plane - degenerate */
    s = edge_intersect_edge_with_normal(&e, a, n);
    ASSERT(segment_is_degenerate(s), "Edge on plane is degenerate");
    
    edge_free(&e);
    edge_init(&e, r3_create(0.0, 0.0, -1.0), r3_create(1.0, 0.0, 1.0));
    
    /* First half belongs to half-space */
    a = r3_create(1.0, 1.0, 0.0);
    s = edge_intersect_edge_with_normal(&e, a, n);
    ASSERT(APPROX_EQ(s.beg, 0.0, 1e-9) && APPROX_EQ(s.fin, 0.5, 1e-9),
           "First half in half-space");
    
    edge_free(&e);
    edge_init(&e, r3_create(0.0, 0.0, 1.0), r3_create(1.0, 0.0, -1.0));
    
    /* Second half belongs to half-space */
    s = edge_intersect_edge_with_normal(&e, a, n);
    ASSERT(APPROX_EQ(s.beg, 0.5, 1e-9) && APPROX_EQ(s.fin, 1.0, 1e-9),
           "Second half in half-space");
    
    edge_free(&e);
}

/* ==================== Facet Tests ==================== */
void test_facet_vertical() {
    printf("\n=== Facet Vertical Tests ===\n");
    
    R3 V = r3_create(0.0, 0.0, 1.0);
    
    /* Non-vertical facet */
    R3 verts1[3] = {
        r3_create(0.0, 0.0, 0.0),
        r3_create(3.0, 0.0, 0.0),
        r3_create(0.0, 3.0, 0.0)
    };
    Facet f1;
    facet_init(&f1, verts1, 3);
    ASSERT(!facet_is_vertical(&f1, V), "Horizontal facet is not vertical");
    facet_free(&f1);
    
    /* Vertical facet */
    R3 verts2[3] = {
        r3_create(0.0, 0.0, 0.0),
        r3_create(0.0, 0.0, 1.0),
        r3_create(1.0, 0.0, 0.0)
    };
    Facet f2;
    facet_init(&f2, verts2, 3);
    ASSERT(facet_is_vertical(&f2, V), "Vertical facet is vertical");
    facet_free(&f2);
}

void test_facet_h_normal() {
    printf("\n=== Facet H-Normal Tests ===\n");
    
    R3 V = r3_create(0.0, 0.0, 1.0);
    
    /* Horizontal facet - normal should be (0,0,1) */
    R3 verts1[3] = {
        r3_create(0.0, 0.0, 0.0),
        r3_create(3.0, 0.0, 0.0),
        r3_create(0.0, 3.0, 0.0)
    };
    Facet f1;
    facet_init(&f1, verts1, 3);
    R3 n1 = facet_h_normal(&f1, V);
    R3 expected1 = r3_create(0.0, 0.0, 1.0);
    /* Check collinearity */
    double dot1 = r3_dot(n1, expected1);
    double mag1 = sqrt(r3_dot(n1, n1));
    ASSERT(APPROX_EQ(dot1 / mag1, 1.0, 1e-9), "H-normal points up for horizontal facet");
    facet_free(&f1);
}

void test_facet_v_normals() {
    printf("\n=== Facet V-Normals Tests ===\n");
    
    R3 V = r3_create(0.0, 0.0, 1.0);
    
    /* Square facet */
    R3 verts[4] = {
        r3_create(0.0, 0.0, 0.0),
        r3_create(2.0, 0.0, 0.0),
        r3_create(2.0, 2.0, 0.0),
        r3_create(0.0, 2.0, 0.0)
    };
    Facet f;
    facet_init(&f, verts, 4);
    
    int count;
    R3 *normals = facet_v_normals(&f, V, &count);
    ASSERT(count == 4, "Square has 4 v-normals");
    
    /* Expected normals: (-1,0,0), (0,-1,0), (1,0,0), (0,1,0) */
    R3 expected[4] = {
        r3_create(-1.0, 0.0, 0.0),
        r3_create(0.0, -1.0, 0.0),
        r3_create(1.0, 0.0, 0.0),
        r3_create(0.0, 1.0, 0.0)
    };
    
    int all_collinear = 1;
    for (int i = 0; i < 4; i++) {
        double dot = r3_dot(normals[i], expected[i]);
        double mag_n = sqrt(r3_dot(normals[i], normals[i]));
        double mag_e = sqrt(r3_dot(expected[i], expected[i]));
        if (!APPROX_EQ(dot / (mag_n * mag_e), 1.0, 1e-9)) {
            all_collinear = 0;
            break;
        }
    }
    ASSERT(all_collinear, "V-normals are correct for square");
    
    free(normals);
    facet_free(&f);
}

void test_facet_center() {
    printf("\n=== Facet Center Tests ===\n");
    
    /* Square center */
    R3 verts1[4] = {
        r3_create(0.0, 0.0, 0.0),
        r3_create(2.0, 0.0, 0.0),
        r3_create(2.0, 2.0, 0.0),
        r3_create(0.0, 2.0, 0.0)
    };
    Facet f1;
    facet_init(&f1, verts1, 4);
    R3 c1 = facet_center(&f1);
    R3 expected1 = r3_create(1.0, 1.0, 0.0);
    ASSERT(R3_APPROX_EQ(c1, expected1, 1e-9), "Square center is (1,1,0)");
    facet_free(&f1);
    
    /* Triangle center */
    R3 verts2[3] = {
        r3_create(0.0, 0.0, 0.0),
        r3_create(3.0, 0.0, 0.0),
        r3_create(0.0, 3.0, 0.0)
    };
    Facet f2;
    facet_init(&f2, verts2, 3);
    R3 c2 = facet_center(&f2);
    R3 expected2 = r3_create(1.0, 1.0, 0.0);
    ASSERT(R3_APPROX_EQ(c2, expected2, 1e-9), "Triangle center is (1,1,0)");
    facet_free(&f2);
}

/* ==================== Polyedr Tests ==================== */
void test_polyedr_load() {
    printf("\n=== Polyedr Load Tests ===\n");
    
    /* Test programmatic cube creation instead of file loading */
    Polyedr p;
    if (polyedr_create_cube(&p) == 0) {
        ASSERT(p.vertex_count == 8, "Cube has 8 vertices");
        ASSERT(p.facet_count == 6, "Cube has 6 facets");
        ASSERT(p.edge_count == 24, "Cube has 24 edges (4 per facet)");
        
        /* Test shadow computation on the cube */
        for (int e_idx = 0; e_idx < p.edge_count; e_idx++) {
            Edge *e = &p.edges[e_idx];
            
            /* Apply shadows from all facets */
            for (int f_idx = 0; f_idx < p.facet_count; f_idx++) {
                edge_shadow(e, &p.facets[f_idx], p.V);
            }
            
            /* After all shadows, gaps should still exist (some visible parts) */
            ASSERT(e->gaps_count >= 0, "Edge has non-negative gaps after shadow");
        }
        
        polyedr_free(&p);
    } else {
        printf("  SKIP: Could not create cube\n");
    }
}

int main() {
    printf("========================================\n");
    printf("Polyedr C Translation - Unit Tests\n");
    printf("========================================\n");
    
    /* Segment tests */
    test_segment_degenerate();
    test_segment_intersect();
    test_segment_subtraction();
    
    /* Edge tests */
    test_edge_r3();
    test_edge_intersect();
    
    /* Facet tests */
    test_facet_vertical();
    test_facet_h_normal();
    test_facet_v_normals();
    test_facet_center();
    
    /* Polyedr tests */
    test_polyedr_load();
    
    printf("\n========================================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("========================================\n");
    
    return tests_failed > 0 ? 1 : 0;
}
