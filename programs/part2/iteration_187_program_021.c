#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations for functions in lib.c */
void hot_function_a(int iterations);
void hot_function_b(int iterations);
void cold_function_c(void);
void medium_function_d(int iterations);
void rarely_called_e(void);
void path_variant_f(int mode);

/* Local functions */
static void local_hot_loop(int count) {
    for (int i = 0; i < count; i++) {
        if (i % 3 == 0) {
            printf("Divisible by 3: %d\n", i);
        } else if (i % 7 == 0) {
            printf("Divisible by 7: %d\n", i);
        }
    }
}

static void local_cold_path(void) {
    printf("This path is rarely executed\n");
    for (int i = 0; i < 5; i++) {
        printf("Cold loop iteration %d\n", i);
    }
}

static void local_conditional(int value) {
    if (value > 100) {
        printf("High value path\n");
        local_hot_loop(50);
    } else if (value > 50) {
        printf("Medium value path\n");
    } else {
        printf("Low value path\n");
        local_cold_path();
    }
}

void main_helper_function(int mode) {
    printf("Main helper executing in mode %d\n", mode);
    
    switch (mode) {
        case 1:
            local_hot_loop(1000);
            break;
        case 2:
            local_conditional(75);
            break;
        case 3:
            local_cold_path();
            break;
        default:
            local_hot_loop(100);
            local_conditional(30);
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 100;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    /* Vary execution based on mode to create different profiles */
    switch (mode) {
        case 1:
            /* Mode 1: Heavy execution of hot functions */
            hot_function_a(iterations * 10);
            hot_function_b(iterations * 5);
            main_helper_function(1);
            break;
            
        case 2:
            /* Mode 2: Balanced execution */
            hot_function_a(iterations);
            medium_function_d(iterations * 2);
            cold_function_c();
            main_helper_function(2);
            break;
            
        case 3:
            /* Mode 3: Light execution with cold paths */
            rarely_called_e();
            cold_function_c();
            main_helper_function(3);
            break;
            
        default:
            /* Default: Mixed execution */
            hot_function_a(iterations);
            hot_function_b(iterations / 2);
            medium_function_d(iterations);
            rarely_called_e();
            main_helper_function(mode);
    }
    
    /* Always call the path variant */
    path_variant_f(mode);
    
    return 0;
}
