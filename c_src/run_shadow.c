#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "polyedr.h"
#include "tk_drawer.h"

int main(int argc, char *argv[]) {
    const char *names[] = {"ccc", "cube", "box", "king", "cow"};
    int num_names = 5;
    
    TkDrawer tk;
    if (tk_init(&tk) != 0) {
        return 1;
    }
    
    for (int i = 0; i < num_names; i++) {
        printf("=============================================================\n");
        printf("Starting work with polyedr '%s'\n", names[i]);
        
        char filename[256];
        snprintf(filename, sizeof(filename), "../data/%s.geom", names[i]);
        
        clock_t start_time = clock();
        
        Polyedr p;
        if (polyedr_init(&p, filename) != 0) {
            fprintf(stderr, "Failed to load polyedr from %s\n", filename);
            continue;
        }
        
        /* Draw the polyedr */
        tk_clean(&tk);
        for (int e_idx = 0; e_idx < p.edge_count; e_idx++) {
            Edge *e = &p.edges[e_idx];
            
            /* Apply shadows from all facets */
            for (int f_idx = 0; f_idx < p.facet_count; f_idx++) {
                edge_shadow(e, &p.facets[f_idx], p.V);
            }
            
            /* Draw visible segments */
            for (int g = 0; g < e->gaps_count; g++) {
                R3 p1 = edge_r3(e, e->gaps[g].beg);
                R3 p2 = edge_r3(e, e->gaps[g].fin);
                tk_draw_line(&tk, p1, p2);
            }
        }
        
        clock_t end_time = clock();
        double delta_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
        printf("Drawing polyedr '%s' took %.6f seconds.\n", names[i], delta_time);
        
        /* Wait for user input */
        printf("Press Enter to continue...\n");
        getchar();
        
        polyedr_free(&p);
    }
    
    tk_close(&tk);
    printf("\nStop\n");
    
    return 0;
}
