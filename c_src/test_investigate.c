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
    double c, alpha, beta, gamma;
    
    /* Allocate arrays */
    R3 *vertexes = NULL;
    Facet *facets = NULL;
    Edge *edges = NULL;
    int edge_count = 0;
    int facet_count = 0;
    
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
        } else if (line_num == 2) {
            sscanf(line, "%d %d %d", &nv, &nf, &ne);
            vertexes = (R3*)malloc(nv * sizeof(R3));
            facets = (Facet*)malloc(nf * sizeof(Facet));
            printf("Allocated %d vertices and %d facets\n", nv, nf);
        } else if (line_num <= nv + 1) {
            double x, y, z;
            sscanf(line, "%lf %lf %lf", &x, &y, &z);
            R3 v = r3_create(x, y, z);
            v = r3_rz(v, alpha);
            v = r3_ry(v, beta);
            v = r3_rz(v, gamma);
            v = r3_mul(v, c);
            vertexes[line_num - 2] = v;
        } else {
            /* Facet */
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
            for (int i = 0; i < size; i++) {
                int idx = atoi(tokens[i + 1]) - 1;
                facet_vertices[i] = vertexes[idx];
            }
            
            /* Create edges */
            for (int n = 0; n < size; n++) {
                Edge new_edge;
                edge_init(&new_edge, facet_vertices[(n - 1 + size) % size], facet_vertices[n]);
                
                edge_count++;
                Edge *temp = (Edge*)realloc(edges, edge_count * sizeof(Edge));
                if (!temp) {
                    fprintf(stderr, "Realloc failed!\n");
                    free(facet_vertices);
                    fclose(file);
                    return 1;
                }
                edges = temp;
                edges[edge_count - 1] = new_edge;
            }
            
            /* Create facet */
            facet_init(&facets[facet_count++], facet_vertices, size);
            free(facet_vertices);
        }
    }
    
    fclose(file);
    
    printf("\nFinal state:\n");
    printf("Vertices: %d\n", nv);
    printf("Facets: %d\n", facet_count);
    printf("Edges: %d\n", edge_count);
    
    /* Print all edge pointers */
    printf("\nEdge details:\n");
    for (int i = 0; i < edge_count; i++) {
        printf("Edge %d: gaps=%d, gaps_ptr=%p\n", 
               i, edges[i].gaps_count, (void*)edges[i].gaps);
    }
    
    /* Now let's manually check what happens when we try to access them */
    printf("\nAccessing edge data:\n");
    for (int i = 0; i < edge_count; i++) {
        Segment *gaps = edges[i].gaps;
        printf("Edge %d gaps[0]: beg=%f, fin=%f\n", i, gaps[0].beg, gaps[0].fin);
    }
    
    printf("\nDone!\n");
    
    /* Free everything manually */
    for (int i = 0; i < edge_count; i++) {
        if (edges[i].gaps) free(edges[i].gaps);
    }
    if (edges) free(edges);
    
    for (int i = 0; i < facet_count; i++) {
        if (facets[i].vertexes) free(facets[i].vertexes);
    }
    if (facets) free(facets);
    
    if (vertexes) free(vertexes);
    
    printf("Freed all memory successfully!\n");
    
    return 0;
}
