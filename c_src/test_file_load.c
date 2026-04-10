#include <stdio.h>
#include <stdlib.h>
#include "polyedr.h"

int main() {
    const char *names[] = {"ccc", "cube", "box"};
    
    for (int i = 0; i < 3; i++) {
        printf("Testing %s...\n", names[i]);
        
        char filename[256];
        snprintf(filename, sizeof(filename), "../data/%s.geom", names[i]);
        
        Polyedr p;
        if (polyedr_init(&p, filename) != 0) {
            fprintf(stderr, "Failed to load %s\n", filename);
            continue;
        }
        
        printf("Loaded %s: %d vertices, %d facets, %d edges\n", 
               names[i], p.vertex_count, p.facet_count, p.edge_count);
        
        /* Test shadow application on first edge */
        if (p.edge_count > 0) {
            Edge *e = &p.edges[0];
            printf("Edge 0: %d gaps before shadows\n", e->gaps_count);
            
            for (int f_idx = 0; f_idx < p.facet_count; f_idx++) {
                edge_shadow(e, &p.facets[f_idx], p.V);
            }
            
            printf("Edge 0: %d gaps after shadows\n", e->gaps_count);
        }
        
        polyedr_free(&p);
        printf("OK\n\n");
    }
    
    return 0;
}
