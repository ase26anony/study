#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib.h"

#define HOT_LOOP_COUNT 100000
#define WARM_LOOP_COUNT 10000
#define COLD_LOOP_COUNT 100

/* Function with varying execution patterns */
void hot_function(int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Hot code path - always executed */
        int x = i * 2;
        if (x % 3 == 0) {
            /* Sometimes taken path */
            x += 1;
        } else {
            /* Alternative path */
            x -= 1;
        }
    }
}

void warm_function(int iterations) {
    for (int i = 0; i < iterations; i++) {
        if (i % 10 == 0) {
            /* Less frequently executed */
            int y = i * 3;
        }
    }
}

void cold_function(int iterations) {
    /* Rarely executed function */
    if (iterations > 0) {
        printf("Cold function executed %d times\n", iterations);
    }
}

void mixed_function(int mode) {
    switch (mode) {
        case 1:
            hot_function(HOT_LOOP_COUNT / 10);
            break;
        case 2:
            warm_function(WARM_LOOP_COUNT / 5);
            break;
        default:
            cold_function(1);
            break;
    }
}

void recursive_function(int depth, int current) {
    if (current >= depth) return;
    
    /* Create some branching */
    if (current % 2 == 0) {
        hot_function(10);
    } else {
        warm_function(5);
    }
    
    recursive_function(depth, current + 1);
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int seed = 42;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    srand(seed);
    
    printf("Running in mode %d with seed %d\n", mode, seed);
    
    /* Vary execution based on mode */
    switch (mode) {
        case 1:  /* Hot profile - focus on hot functions */
            hot_function(HOT_LOOP_COUNT);
            warm_function(WARM_LOOP_COUNT);
            lib_hot_function(HOT_LOOP_COUNT / 2);
            lib_warm_function(WARM_LOOP_COUNT / 2);
            mixed_function(1);
            recursive_function(50, 0);
            break;
            
        case 2:  /* Warm profile - more balanced */
            hot_function(HOT_LOOP_COUNT / 2);
            warm_function(WARM_LOOP_COUNT * 2);
            cold_function(5);
            lib_hot_function(HOT_LOOP_COUNT / 4);
            lib_warm_function(WARM_LOOP_COUNT);
            lib_cold_function(3);
            mixed_function(2);
            recursive_function(30, 0);
            break;
            
        case 3:  /* Cold profile - focus on rarely used code */
            hot_function(COLD_LOOP_COUNT);
            warm_function(COLD_LOOP_COUNT * 2);
            cold_function(10);
            lib_hot_function(COLD_LOOP_COUNT);
            lib_cold_function(8);
            mixed_function(3);
            recursive_function(10, 0);
            break;
            
        default: /* Mixed profile */
            for (int i = 0; i < mode; i++) {
                hot_function(1000);
                if (i % 3 == 0) {
                    warm_function(500);
                }
                if (i % 7 == 0) {
                    cold_function(1);
                }
            }
            break;
    }
    
    /* Call library functions */
    lib_mixed_function(mode);
    
    return 0;
}
