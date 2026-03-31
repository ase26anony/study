#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"

#define HOT_LOOP_COUNT 1000
#define COLD_LOOP_COUNT 10

/* Function with high execution frequency */
void hot_function_a(int iterations) {
    int i, sum = 0;
    for (i = 0; i < iterations; i++) {
        sum += i;
        if (i % 100 == 0) {
            sum -= 50;  /* Cold path inside hot loop */
        }
    }
    printf("Hot function A result: %d\n", sum);
}

/* Function with medium execution frequency */
void medium_function_b(int iterations) {
    int i;
    for (i = 0; i < iterations / 2; i++) {
        if (i % 3 == 0) {
            printf("Divisible by 3: %d\n", i);
        }
    }
}

/* Function that's rarely called */
void cold_function_c(void) {
    printf("This function is rarely executed\n");
    /* Some branching logic */
    int x = rand() % 10;
    if (x < 5) {
        printf("Path A\n");
    } else {
        printf("Path B\n");
    }
}

/* Function that calls library functions */
void mixed_function_d(int mode) {
    if (mode == 1) {
        lib_function1(HOT_LOOP_COUNT);
        lib_function3(50);
    } else {
        lib_function2(COLD_LOOP_COUNT);
        cold_function_c();
    }
}

/* Main driver with different execution modes */
int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = HOT_LOOP_COUNT;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode %d\n", mode);
    
    switch (mode) {
        case 1:
            /* Mode 1: Heavy execution of hot paths */
            for (int i = 0; i < 5; i++) {
                hot_function_a(iterations);
                medium_function_b(iterations);
                lib_function1(iterations / 2);
            }
            mixed_function_d(1);
            break;
            
        case 2:
            /* Mode 2: Balanced execution */
            for (int i = 0; i < 3; i++) {
                hot_function_a(iterations / 2);
                medium_function_b(iterations);
            }
            lib_function2(iterations);
            mixed_function_d(2);
            cold_function_c();
            break;
            
        case 3:
            /* Mode 3: Light execution with cold paths */
            hot_function_a(COLD_LOOP_COUNT);
            medium_function_b(COLD_LOOP_COUNT * 2);
            for (int i = 0; i < 3; i++) {
                cold_function_c();
            }
            lib_function3(100);
            break;
            
        default:
            /* Default: Mix of everything */
            hot_function_a(100);
            medium_function_b(200);
            lib_function1(150);
            lib_function2(75);
            lib_function3(50);
            cold_function_c();
    }
    
    /* Always execute this path */
    printf("Program completed in mode %d\n", mode);
    
    return 0;
}
