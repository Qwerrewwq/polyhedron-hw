#include <stdio.h>
#include <stdlib.h>
#include "polyedr.h"

int main() {
    const char *filename = "../data/ccc.geom";
    
    printf("Testing %s...\n", filename);
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Cannot open file\n");
        return 1;
    }
    
    char line[1024];
    int line_num = 0;
    int nv = 0, nf = 0, ne = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        if (line_num == 2) {
            sscanf(line, "%d %d %d", &nv, &nf, &ne);
            printf("nv=%d, so vertex lines: 3 to %d\n", nv, nv + 1);
            printf("Condition line_num < nv + 2 means: line_num < %d\n", nv + 2);
        }
        
        if (line_num >= 3 && line_num < nv + 2) {
            printf("Line %d is a VERTEX line\n", line_num);
        } else if (line_num >= nv + 2) {
            printf("Line %d is a FACET line\n", line_num);
        }
    }
    
    fclose(file);
    return 0;
}
