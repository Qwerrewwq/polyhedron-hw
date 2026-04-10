#include <stdio.h>
#include <stdlib.h>
#include "polyedr.h"

int main() {
    const char *filename = "../data/ccc.geom";
    
    printf("Testing %s...\n", filename);
    
    Polyedr p;
    int result = polyedr_init(&p, filename);
    printf("polyedr_init returned: %d\n", result);
    
    if (result != 0) {
        fprintf(stderr, "Failed to load %s\n", filename);
        return 1;
    }
    
    printf("Loaded: %d vertices, %d facets, %d edges\n", 
           p.vertex_count, p.facet_count, p.edge_count);
    
    printf("Done!\n");
    
    return 0;
}
