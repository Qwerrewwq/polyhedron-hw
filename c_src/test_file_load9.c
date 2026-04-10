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
    double c = 1.0;
    double alpha = 0.0, beta = 0.0, gamma = 0.0;
    int nv = 0, nf = 0, ne = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        if (line_num == 1) {
            char *token = strtok(line, " \t\n");
            if (token) c = atof(token);
            token = strtok(NULL, " \t\n");
            if (token) alpha = atof(token) * 3.14159265358979323846 / 180.0;
            token = strtok(NULL, " \t\n");
            if (token) beta = atof(token) * 3.14159265358979323846 / 180.0;
            token = strtok(NULL, " \t\n");
            if (token) gamma = atof(token) * 3.14159265358979323846 / 180.0;
            printf("Parsed: c=%f, alpha=%f, beta=%f, gamma=%f\n", c, alpha, beta, gamma);
        } else if (line_num == 2) {
            sscanf(line, "%d %d %d", &nv, &nf, &ne);
            printf("Parsed: nv=%d, nf=%d, ne=%d\n", nv, nf, ne);
        } else if (line_num < nv + 2) {
            double x, y, z;
            sscanf(line, "%lf %lf %lf", &x, &y, &z);
            printf("Vertex %d: (%f, %f, %f)\n", line_num - 2, x, y, z);
        } else {
            printf("Facet line: %s", line);
            char *tokens[64];
            int token_count = 0;
            char *token = strtok(line, " \t\n");
            while (token && token_count < 64) {
                tokens[token_count++] = token;
                token = strtok(NULL, " \t\n");
            }
            
            if (token_count < 1) continue;
            
            int size = atoi(tokens[0]);
            printf("  Facet with %d vertices:", size);
            for (int i = 0; i < size; i++) {
                int idx = atoi(tokens[i + 1]) - 1;
                printf(" %d", idx);
            }
            printf("\n");
        }
    }
    
    fclose(file);
    printf("Done!\n");
    
    return 0;
}
