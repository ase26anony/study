#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib.h"

#define HOT_LOOP_COUNT 100000
#define COLD_LOOP_COUNT 100

/* Function with varying execution patterns */
void hot_function(int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Hot code path - executed many times */
        if (i % 2 == 0) {
            /* Even path */
            volatile int x = i * 2;
        } else {
            /* Odd path */
            volatile int y = i / 2;
        }
    }
}

void cold_function(int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Cold code path - executed rarely */
        if (i < 10) {
            volatile int z = i * i;
        }
    }
}

void mixed_function(int mode) {
    if (mode == 1) {
        /* Hot path for mode 1 */
        for (int i = 0; i < HOT_LOOP_COUNT / 10; i++) {
            volatile int temp = i % 7;
        }
    } else {
        /* Cold path for other modes */
        for (int i = 0; i < 5; i++) {
            volatile int temp = i;
        }
    }
}

void recursive_function(int depth, int current) {
    if (current >= depth) return;
    
    volatile int local = current * 2;
    if (current % 3 == 0) {
        recursive_function(depth, current + 1);
    } else {
        recursive_function(depth, current + 2);
    }
}

void switch_based_function(int value) {
    switch (value % 4) {
        case 0:
            /* Frequently executed case */
            volatile int a = value;
            break;
        case 1:
            /* Less frequent case */
            volatile int b = value * 2;
            break;
        case 2:
            /* Rare case */
            volatile int c = value / 2;
            break;
        default:
            /* Default case */
            volatile int d = value + 1;
            break;
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode %d\n", mode);
    
    /* Vary execution based on mode to create different profiles */
    if (mode == 1) {
        /* Mode 1: Heavy execution of hot paths */
        hot_function(HOT_LOOP_COUNT);
        cold_function(COLD_LOOP_COUNT);
        mixed_function(1);
        recursive_function(10, 0);
        
        /* Call library functions */
        lib_hot_function(HOT_LOOP_COUNT / 2);
        lib_cold_function(5);
        lib_complex_function(1);
        
        /* Execute switch function many times */
        for (int i = 0; i < HOT_LOOP_COUNT / 100; i++) {
            switch_based_function(i);
        }
        
    } else if (mode == 2) {
        /* Mode 2: Different execution pattern */
        hot_function(HOT_LOOP_COUNT / 10);
        cold_function(COLD_LOOP_COUNT * 2);
        mixed_function(2);
        recursive_function(5, 0);
        
        /* Different library usage */
        lib_hot_function(HOT_LOOP_COUNT / 20);
        lib_cold_function(20);
        lib_complex_function(2);
        
        /* Different switch pattern */
        for (int i = 0; i < HOT_LOOP_COUNT / 1000; i++) {
            switch_based_function(i * 3);
        }
        
    } else {
        /* Mode 3: Minimal execution */
        hot_function(100);
        cold_function(10);
        mixed_function(3);
        recursive_function(3, 0);
        
        lib_hot_function(50);
        lib_cold_function(3);
        lib_complex_function(3);
    }
    
    /* Always execute this path */
    volatile int final_result = mode * 100;
    
    return 0;
}
