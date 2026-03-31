#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"

#define HOT_LOOP_COUNT 1000
#define COLD_LOOP_COUNT 10
#define VERY_COLD_LOOP_COUNT 2

/* Function with varying execution frequency based on mode */
void process_data(int mode, int iterations) {
    int i, j;
    int sum = 0;
    
    if (mode == 1) {
        /* Hot path - executed many times */
        for (i = 0; i < iterations; i++) {
            for (j = 0; j < 100; j++) {
                sum += i * j;
            }
        }
        printf("Processed data with sum: %d\n", sum);
    } else if (mode == 2) {
        /* Medium path */
        for (i = 0; i < iterations / 2; i++) {
            sum += i * i;
        }
        printf("Processed data with sum: %d\n", sum);
    } else {
        /* Cold path */
        sum = iterations;
        printf("Processed data with sum: %d\n", sum);
    }
}

/* Another function with conditional execution */
void analyze_results(int mode) {
    int i;
    double result = 0.0;
    
    switch (mode) {
        case 1:
            /* Hot analysis */
            for (i = 0; i < HOT_LOOP_COUNT; i++) {
                result += (double)i / (i + 1);
            }
            break;
        case 2:
            /* Medium analysis */
            for (i = 0; i < COLD_LOOP_COUNT; i++) {
                result += (double)i * 0.5;
            }
            break;
        default:
            /* Cold analysis */
            result = 1.0;
            break;
    }
    
    printf("Analysis result: %.2f\n", result);
}

/* Function that calls library functions */
void perform_calculations(int mode) {
    int i;
    
    if (mode == 1) {
        /* Hot calculations */
        for (i = 0; i < HOT_LOOP_COUNT; i++) {
            int val = lib_compute(i, i * 2);
            if (val > 1000) {
                lib_process_large(val);
            } else {
                lib_process_small(val);
            }
        }
    } else if (mode == 2) {
        /* Medium calculations */
        for (i = 0; i < COLD_LOOP_COUNT; i++) {
            lib_compute(i, i + 1);
        }
    } else {
        /* Cold calculations */
        lib_compute(1, 2);
    }
}

/* Function with nested conditionals */
void handle_user_input(int input, int mode) {
    if (input > 0) {
        if (mode == 1) {
            printf("Positive input in mode 1\n");
            for (int i = 0; i < HOT_LOOP_COUNT; i++) {
                /* Do some work */
            }
        } else if (mode == 2) {
            printf("Positive input in mode 2\n");
            for (int i = 0; i < COLD_LOOP_COUNT; i++) {
                /* Do less work */
            }
        } else {
            printf("Positive input in other mode\n");
        }
    } else if (input < 0) {
        printf("Negative input\n");
        lib_handle_negative(input);
    } else {
        printf("Zero input\n");
        /* Rarely executed */
    }
}

/* Main entry point with mode selection */
int main(int argc, char *argv[]) {
    int mode = 1;  /* Default mode */
    int iterations = 100;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    /* Execute functions with different frequencies based on mode */
    process_data(mode, iterations);
    analyze_results(mode);
    perform_calculations(mode);
    
    /* Vary input based on mode */
    for (int i = 0; i < iterations; i++) {
        handle_user_input((i % 10) - 5, mode);
    }
    
    /* Call some library functions directly */
    if (mode == 1) {
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            lib_utility_function(i);
        }
    } else {
        lib_utility_function(1);
    }
    
    printf("Program completed successfully\n");
    return 0;
}
