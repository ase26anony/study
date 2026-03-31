#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib.h"

#define HOT_LOOP_COUNT 100000
#define COLD_LOOP_COUNT 100

/* Function with varying execution patterns */
void hot_function(int iterations) {
    int i;
    for (i = 0; i < iterations; i++) {
        /* Hot path - always executed */
        if (i % 2 == 0) {
            /* Even iteration path */
            volatile int x = i * 2;
        } else {
            /* Odd iteration path */
            volatile int y = i * 3;
        }
        
        /* Nested condition for coverage */
        if (i % 10 == 0) {
            /* Less frequent path */
            volatile int z = i / 10;
        }
    }
}

void cold_function(int iterations) {
    int i;
    for (i = 0; i < iterations; i++) {
        /* Cold path - rarely executed */
        if (i % 50 == 0) {
            volatile int a = i * 5;
        }
    }
}

void mixed_function(int mode) {
    /* Function with different execution patterns based on mode */
    switch (mode) {
        case 1:
            /* Mode 1: Execute hot path */
            hot_function(HOT_LOOP_COUNT / 10);
            break;
        case 2:
            /* Mode 2: Execute cold path */
            cold_function(COLD_LOOP_COUNT);
            break;
        case 3:
            /* Mode 3: Mix of both */
            hot_function(HOT_LOOP_COUNT / 20);
            cold_function(COLD_LOOP_COUNT * 2);
            break;
        default:
            /* Default: minimal execution */
            hot_function(10);
            cold_function(5);
    }
}

void recursive_function(int depth, int max_depth) {
    /* Recursive function for varied call graphs */
    if (depth >= max_depth) {
        return;
    }
    
    volatile int local = depth * 2;
    
    /* Sometimes call recursively, sometimes not */
    if (depth % 3 == 0) {
        recursive_function(depth + 1, max_depth);
    }
    
    /* Call library function */
    lib_function_a(depth);
}

void data_dependent_function(int* data, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (data[i] > 100) {
            /* Hot threshold path */
            volatile int hot = data[i] * 2;
        } else if (data[i] > 50) {
            /* Warm path */
            volatile int warm = data[i] + 10;
        } else {
            /* Cold path */
            volatile int cold = data[i] - 5;
        }
    }
}

int main(int argc, char** argv) {
    int mode = 1;
    int i;
    
    /* Parse mode from command line */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode %d\n", mode);
    
    /* Seed random for varied execution */
    srand(time(NULL) + mode);
    
    /* Execute different patterns based on mode */
    switch (mode) {
        case 1:
            /* Mode 1: Heavy hot execution */
            for (i = 0; i < 5; i++) {
                hot_function(HOT_LOOP_COUNT);
                mixed_function(1);
            }
            recursive_function(0, 10);
            break;
            
        case 2:
            /* Mode 2: Balanced execution */
            for (i = 0; i < 3; i++) {
                hot_function(HOT_LOOP_COUNT / 2);
                cold_function(COLD_LOOP_COUNT * 3);
                mixed_function(3);
            }
            recursive_function(0, 5);
            break;
            
        case 3:
            /* Mode 3: Light execution with more cold paths */
            hot_function(HOT_LOOP_COUNT / 100);
            for (i = 0; i < 10; i++) {
                cold_function(COLD_LOOP_COUNT);
                mixed_function(2);
            }
            recursive_function(0, 3);
            break;
            
        default:
            /* Default: Mixed execution */
            hot_function(HOT_LOOP_COUNT / 10);
            cold_function(COLD_LOOP_COUNT);
            mixed_function(mode % 3 + 1);
    }
    
    /* Call library functions with different frequencies */
    lib_function_a(mode * 10);
    lib_function_b(mode * 20);
    
    if (mode % 2 == 0) {
        lib_function_c(mode * 30);
    }
    
    if (mode > 1) {
        lib_complex_function(mode, mode * 2);
    }
    
    /* Data-dependent execution */
    int data[100];
    for (i = 0; i < 100; i++) {
        data[i] = rand() % 200;
    }
    data_dependent_function(data, 100);
    
    /* Final call to ensure all functions are at least referenced */
    lib_utility_function();
    
    printf("Mode %d execution complete\n", mode);
    return 0;
}
