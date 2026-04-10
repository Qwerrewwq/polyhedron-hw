#include <stdio.h>
#include <stdlib.h>
#include "polyedr.h"

int main() {
    const char *filename = "../data/ccc.geom";
    
    printf("Testing %s...\n", filename);
    
    Polyedr p;
    if (polyedr_init(&p, filename) != 0) {
        fprintf(stderr, "Failed to load %s\n", filename);
        return 1;
    }
    
    printf("Loaded: %d vertices, %d facets, %d edges\n", 
           p.vertex_count, p.facet_count, p.edge_count);
    
    /* Print edge info */
    for (int i = 0; i < p.edge_count; i++) {
        Edge *e = &p.edges[i];
        printf("Edge %d: gaps=%d, gaps_capacity=%d, gaps_ptr=%p\n", 
               i, e->gaps_count, e->gaps_capacity, (void*)e->gaps);
    }
    
    printf("About to free edges...\n");
    for (int i = 0; i < p.edge_count; i++) {
        printf("Freeing edge %d, gaps_ptr=%p\n", i, (void*)p.edges[i].gaps);
        edge_free(&p.edges[i]);
    }
    printf("Freed all edges\n");
    
    if (p.edges) {
        free(p.edges);
        p.edges = NULL;
    }
    
    if (p.facets) {
        for (int i = 0; i < p.facet_count; i++) {
            facet_free(&p.facets[i]);
        }
        free(p.facets);
        p.facets = NULL;
    }
    
    if (p.vertexes) {
        free(p.vertexes);
        p.vertexes = NULL;
    }
    
    printf("OK\n");
    
    return 0;
}
