#include <stdio.h>
#include <stdlib.h>
#include "polyedr.h"

int main() {
    printf("Testing cube creation...\n");
    
    Polyedr p;
    if (polyedr_create_cube(&p) != 0) {
        fprintf(stderr, "Failed to create cube\n");
        return 1;
    }
    
    printf("Created cube with %d vertices, %d facets, %d edges\n", 
           p.vertex_count, p.facet_count, p.edge_count);
    
    /* Test shadow application */
    for (int e_idx = 0; e_idx < p.edge_count; e_idx++) {
        Edge *e = &p.edges[e_idx];
        printf("Edge %d: %d gaps before shadows\n", e_idx, e->gaps_count);
        
        for (int f_idx = 0; f_idx < p.facet_count; f_idx++) {
            edge_shadow(e, &p.facets[f_idx], p.V);
        }
        
        printf("Edge %d: %d gaps after shadows\n", e_idx, e->gaps_count);
    }
    
    polyedr_free(&p);
    printf("Test completed successfully!\n");
    return 0;
}
