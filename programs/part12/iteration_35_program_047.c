#include <stdio.h>
#include <stdlib.h>
#include "funcs.h"

/* Global variables to create different execution patterns */
static int global_counter = 0;
static int hot_loop_counter = 0;

/* Function with multiple branches */
void process_value(int value) {
    if (value < 0) {
        printf("Negative value: %d\n", value);
        func1();
    } else if (value == 0) {
        printf("Zero value\n");
        func2();
    } else if (value < 100) {
        printf("Small positive: %d\n", value);
        func3();
    } else {
        printf("Large value: %d\n", value);
        func4();
    }
    
    /* Nested condition for more coverage */
    if (value % 2 == 0) {
        printf("Even number\n");
    } else {
        printf("Odd number\n");
    }
}

/* Function to create hot blocks */
void execute_hot_blocks(int iterations) {
    for (int i = 0; i < iterations; i++) {
        hot_loop_counter++;
        
        /* Create different execution frequencies */
        if (i % 10 == 0) {
            func1();  /* Called 10% of iterations */
        }
        if (i % 50 == 0) {
            func2();  /* Called 2% of iterations */
        }
        if (i % 100 == 0) {
            func3();  /* Called 1% of iterations */
        }
    }
}

/* Function with switch statement */
void process_mode(int mode) {
    switch (mode) {
        case 1:
            printf("Mode 1: Basic processing\n");
            func1();
            break;
        case 2:
            printf("Mode 2: Advanced processing\n");
            func2();
            func3();
            break;
        case 3:
            printf("Mode 3: Full processing\n");
            func1();
            func2();
            func3();
            func4();
            break;
        default:
            printf("Unknown mode: %d\n", mode);
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <value> <mode> [iterations]\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    int mode = atoi(argv[2]);
    int iterations = (argc > 3) ? atoi(argv[3]) : 1000;
    
    printf("Starting with value=%d, mode=%d, iterations=%d\n", 
           value, mode, iterations);
    
    /* Process based on input */
    process_value(value);
    process_mode(mode);
    
    /* Execute hot blocks if iterations > 0 */
    if (iterations > 0) {
        execute_hot_blocks(iterations);
    }
    
    /* Final function call based on global state */
    if (global_counter > 1000) {
        func4();
    }
    
    printf("Final counters: global=%d, hot=%d\n", 
           global_counter, hot_loop_counter);
    
    return 0;
}
