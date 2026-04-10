#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common/tk_drawer.h"
#include "shadow/polyedr.h"

int main(void) {
    const char *names[] = {"ccc", "cube", "box", "king", "cow"};
    int num_names = 5;
    
    TkDrawer *tk = tk_create();
    if (!tk) {
        fprintf(stderr, "Не удалось создать графический интерфейс\n");
        return 1;
    }
    
    for (int i = 0; i < num_names; i++) {
        printf("=============================================================\n");
        printf("Начало работы с полиэдром '%s'\n", names[i]);
        
        char filename[256];
        snprintf(filename, sizeof(filename), "../data/%s.geom", names[i]);
        
        clock_t start_time = clock();
        
        Polyedr *p = polyedr_create(filename);
        if (!p) {
            fprintf(stderr, "Не удалось загрузить полиэдр из файла '%s'\n", filename);
            continue;
        }
        
        polyedr_draw(p, tk);
        
        clock_t end_time = clock();
        double delta_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
        
        printf("Изображение полиэдра '%s' заняло %f сек.\n", names[i], delta_time);
        
        /* Ожидание нажатия клавиши */
        printf("Hit 'Return' to continue -> ");
        fflush(stdout);
        getchar();
        
        polyedr_free(p);
    }
    
    tk_close(tk);
    printf("\nStop\n");
    
    return 0;
}
