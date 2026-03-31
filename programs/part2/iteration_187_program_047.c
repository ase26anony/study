#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib.h"

/* Function with varying execution paths */
void hot_function(int iterations) {
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            printf("Even iteration: %d\n", i);
        } else {
            printf("Odd iteration: %d\n", i);
        }
    }
}

void cold_function(int mode) {
    if (mode == 1) {
        printf("Cold path A executed\n");
    } else if (mode == 2) {
        printf("Cold path B executed\n");
    } else {
        printf("Cold path C executed (rare)\n");
    }
}

void medium_function(int count) {
    int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += i * 2;
        if (sum > 1000) {
            sum = 0;  /* Reset condition */
        }
    }
    printf("Medium function sum: %d\n", sum);
}

void conditional_function(int threshold) {
    if (threshold > 50) {
        printf("High threshold branch\n");
        for (int i = 0; i < 10; i++) {
            printf("Loop %d\n", i);
        }
    } else if (threshold > 20) {
        printf("Medium threshold branch\n");
    } else {
        printf("Low threshold branch\n");
    }
}

void recursive_function(int depth, int current) {
    if (current >= depth) {
        return;
    }
    printf("Recursion level: %d\n", current);
    recursive_function(depth, current + 1);
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
    
    /* Vary execution based on mode */
    switch (mode) {
        case 1:
            /* Mode 1: Heavy execution of hot paths */
            hot_function(iterations * 10);
            medium_function(iterations);
            lib_function_a(iterations / 2);
            lib_function_c(5);  /* Cold */
            break;
            
        case 2:
            /* Mode 2: Different execution pattern */
            hot_function(iterations / 2);
            cold_function(mode);
            lib_function_b(iterations * 2);
            lib_function_d(3);  /* Rare */
            break;
            
        case 3:
            /* Mode 3: Mixed execution */
            hot_function(iterations);
            conditional_function(75);  /* High threshold */
            lib_function_a(iterations);
            lib_function_e(iterations / 4);
            break;
            
        default:
            /* Default: Minimal execution */
            cold_function(0);  /* Rare path */
            lib_function_d(1);
            break;
    }
    
    /* Always execute these but with varying frequency */
    recursive_function(mode * 2, 0);
    
    /* Call library functions */
    lib_utility_function(mode * 10);
    
    return 0;
}
