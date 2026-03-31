#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib.h"

/* Hot function - executed many times */
void hot_function_1(int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Do some work */
        volatile int x = i * i;
    }
}

/* Cold function - executed rarely */
void cold_function_1(int mode) {
    if (mode == 1) {
        printf("Cold path 1\n");
    } else {
        printf("Cold path 2\n");
    }
}

/* Medium frequency function */
void medium_function(int count) {
    for (int i = 0; i < count; i++) {
        if (i % 2 == 0) {
            volatile int y = i * 2;
        } else {
            volatile int y = i * 3;
        }
    }
}

/* Function with multiple paths */
void multi_path_function(int value) {
    if (value < 10) {
        printf("Value < 10\n");
    } else if (value < 50) {
        printf("Value < 50\n");
    } else if (value < 100) {
        printf("Value < 100\n");
    } else {
        printf("Value >= 100\n");
    }
}

/* Recursive function */
int recursive_function(int n, int depth) {
    if (depth <= 0) return n;
    if (n <= 1) return 1;
    return recursive_function(n - 1, depth - 1) + 
           recursive_function(n - 2, depth - 1);
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
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
            /* Mode 1: Heavy on hot_function_1 and lib_function_a */
            hot_function_1(iterations * 10);
            lib_function_a(iterations);
            medium_function(iterations / 2);
            multi_path_function(5);  /* Takes first path */
            break;
            
        case 2:
            /* Mode 2: More balanced execution */
            hot_function_1(iterations);
            lib_function_b(iterations * 2);
            cold_function_1(2);
            medium_function(iterations);
            multi_path_function(75);  /* Takes third path */
            break;
            
        case 3:
            /* Mode 3: Heavy recursion and cold paths */
            recursive_function(10, 3);
            lib_function_c(iterations / 10);
            cold_function_1(1);
            multi_path_function(150);  /* Takes last path */
            break;
            
        default:
            /* Default: Mix of everything */
            hot_function_1(iterations * 5);
            lib_function_a(iterations);
            lib_function_b(iterations);
            medium_function(iterations * 3);
            multi_path_function(25);  /* Takes second path */
    }
    
    /* Call functions from lib.c */
    lib_helper_function(mode * 10);
    
    return 0;
}
