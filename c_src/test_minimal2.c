#include <stdio.h>
#include <stdlib.h>
#include "polyedr.h"

int main() {
    const char *filename = "../data/ccc.geom";
    
    printf("Testing %s...\n", filename);
    
    Polyedr p;
    int result = polyedr_init(&p, filename);
    printf("polyedr_init returned: %d\n", result);
    
    if (result == 0) {
        printf("Successfully loaded!\n");
        printf("Vertices: %d, Facets: %d, Edges: %d\n", 
               p.vertex_count, p.facet_count, p.edge_count);
        
        /* Check first facet */
        if (p.facet_count > 0) {
            printf("First facet has %d vertices, vertexes ptr=%p\n", 
                   p.facets[0].vertex_count, (void*)p.facets[0].vertexes);
        }
        
        /* Check first edge */
        if (p.edge_count > 0) {
            printf("First edge: gaps=%d, gaps_ptr=%p\n", 
                   p.edges[0].gaps_count, (void*)p.edges[0].gaps);
        }
        
        printf("Done without freeing anything!\n");
    }
    
    return 0;
}
