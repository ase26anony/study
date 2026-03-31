#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations for functions in lib.c */
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void rarely_called(void);
void varying_function(int mode);

/* Functions in main.c */
void process_data(int size, int mode) {
    int i;
    if (mode == 1) {
        /* Hot path - executed many times */
        for (i = 0; i < size * 100; i++) {
            if (i % 100 == 0) {
                printf("Processing data point %d\n", i);
            }
        }
    } else if (mode == 2) {
        /* Medium path */
        for (i = 0; i < size * 10; i++) {
            if (i % 50 == 0) {
                printf("Medium processing %d\n", i);
            }
        }
    } else {
        /* Cold path */
        printf("Minimal processing for mode %d\n", mode);
    }
}

void analyze_results(int depth) {
    int i, j;
    if (depth > 0) {
        for (i = 0; i < depth; i++) {
            for (j = 0; j < i; j++) {
                /* Nested loop - creates interesting profile */
                if ((i + j) % 3 == 0) {
                    printf("Analysis: i=%d, j=%d\n", i, j);
                }
            }
        }
    } else {
        printf("No analysis needed\n");
    }
}

void handle_mode(int mode) {
    switch (mode) {
        case 1:
            hot_function(1000);
            process_data(50, 1);
            break;
        case 2:
            hot_function(100);
            process_data(10, 2);
            cold_function();
            break;
        case 3:
            medium_function(50);
            process_data(5, 3);
            rarely_called();
            break;
        default:
            printf("Unknown mode: %d\n", mode);
            break;
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d for %d iterations\n", mode, iterations);
    
    srand(time(NULL));
    
    for (int i = 0; i < iterations; i++) {
        handle_mode(mode);
        analyze_results(mode * 10);
        varying_function(mode);
        
        /* Add some randomness to execution paths */
        if (rand() % 100 < 30) {
            rarely_called();
        }
    }
    
    return 0;
}
