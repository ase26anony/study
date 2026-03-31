#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib.h"

#define HOT_LOOP_COUNT 100000
#define COLD_LOOP_COUNT 100

/* Function with varying execution patterns */
void hot_function(int iterations) {
    int i, sum = 0;
    for (i = 0; i < iterations; i++) {
        sum += i % 100;
        if (i % 1000 == 0) {
            sum += 1;  /* Rarely executed branch */
        }
    }
    printf("Hot function sum: %d\n", sum);
}

void cold_function(int iterations) {
    int i, sum = 0;
    for (i = 0; i < iterations; i++) {
        sum += i % 10;
    }
    printf("Cold function sum: %d\n", sum);
}

void mixed_function(int mode) {
    if (mode == 1) {
        /* Hot path for mode 1 */
        for (int i = 0; i < HOT_LOOP_COUNT / 10; i++) {
            volatile int x = i * i;
        }
    } else {
        /* Cold path for other modes */
        for (int i = 0; i < COLD_LOOP_COUNT; i++) {
            volatile int x = i % 10;
        }
    }
}

void recursive_function(int depth, int max_depth) {
    if (depth >= max_depth) return;
    
    /* Create some work */
    volatile int x = depth * depth;
    
    /* Recursive calls with varying frequency */
    if (depth % 2 == 0) {
        recursive_function(depth + 1, max_depth);
    } else {
        recursive_function(depth + 1, max_depth);
        recursive_function(depth + 1, max_depth);  /* Double call for odd depths */
    }
}

void switch_based_function(int mode) {
    switch (mode) {
        case 1:
            /* Frequently executed case */
            for (int i = 0; i < HOT_LOOP_COUNT / 100; i++) {
                volatile int x = i;
            }
            break;
        case 2:
            /* Less frequent case */
            for (int i = 0; i < COLD_LOOP_COUNT * 2; i++) {
                volatile int x = i * 2;
            }
            break;
        case 3:
            /* Rare case */
            printf("Case 3 executed\n");
            break;
        default:
            /* Default case */
            break;
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = HOT_LOOP_COUNT;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    /* Vary execution based on mode */
    if (mode == 1) {
        /* Mode 1: Heavy execution */
        hot_function(iterations);
        hot_function(iterations / 2);
        lib_hot_function(iterations);
        lib_cold_function(COLD_LOOP_COUNT);
    } else if (mode == 2) {
        /* Mode 2: Balanced execution */
        hot_function(iterations / 10);
        cold_function(iterations / 5);
        lib_hot_function(iterations / 20);
        lib_cold_function(iterations / 100);
    } else if (mode == 3) {
        /* Mode 3: Light execution with recursion */
        cold_function(COLD_LOOP_COUNT);
        recursive_function(0, 5);
        lib_cold_function(COLD_LOOP_COUNT * 2);
    }
    
    /* Always execute these */
    mixed_function(mode);
    switch_based_function(mode);
    
    /* Call library functions */
    lib_mixed_function(mode);
    lib_recursive_function(0, 3);
    
    return 0;
}
