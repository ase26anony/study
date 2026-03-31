#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Forward declarations for functions in lib.c
void hot_function_a(int iterations);
void hot_function_b(int iterations);
void cold_function_c(void);
void medium_function_d(int iterations);
void rarely_called_e(void);
void path_selector(int mode);

// Functions in main.c
void process_data(int count) {
    for (int i = 0; i < count; i++) {
        if (i % 3 == 0) {
            printf("Processing divisible by 3: %d\n", i);
        } else if (i % 7 == 0) {
            printf("Processing divisible by 7: %d\n", i);
        }
    }
}

void analyze_results(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i;
        if (i % 100 == 0) {
            printf("Intermediate sum at %d: %d\n", i, sum);
        }
    }
    printf("Final sum: %d\n", sum);
}

void conditional_execution(int mode) {
    switch (mode) {
        case 1:
            printf("Mode 1: Fast path\n");
            for (int i = 0; i < 50; i++) {
                printf("Fast iteration %d\n", i);
            }
            break;
        case 2:
            printf("Mode 2: Medium path\n");
            for (int i = 0; i < 200; i++) {
                if (i % 2 == 0) {
                    printf("Even medium iteration %d\n", i);
                }
            }
            break;
        case 3:
            printf("Mode 3: Slow path\n");
            for (int i = 0; i < 500; i++) {
                if (i % 3 == 0) {
                    printf("Slow divisible by 3: %d\n", i);
                } else if (i % 5 == 0) {
                    printf("Slow divisible by 5: %d\n", i);
                }
            }
            break;
        default:
            printf("Default mode\n");
            for (int i = 0; i < 10; i++) {
                printf("Default iteration %d\n", i);
            }
    }
}

void recursive_function(int depth, int max_depth) {
    if (depth >= max_depth) {
        printf("Max depth reached: %d\n", depth);
        return;
    }
    
    if (depth % 2 == 0) {
        printf("Even depth: %d\n", depth);
    }
    
    recursive_function(depth + 1, max_depth);
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
    
    // Call functions with different frequencies based on mode
    switch (mode) {
        case 1:
            // Hot path - execute hot functions many times
            for (int i = 0; i < 5; i++) {
                hot_function_a(iterations);
                hot_function_b(iterations / 2);
            }
            process_data(iterations * 2);
            break;
            
        case 2:
            // Medium path - mix of hot and cold
            hot_function_a(iterations / 2);
            cold_function_c();
            medium_function_d(iterations);
            process_data(iterations);
            break;
            
        case 3:
            // Cold path - mostly cold functions
            cold_function_c();
            rarely_called_e();
            process_data(10);
            break;
            
        default:
            // Default - balanced execution
            hot_function_a(iterations / 3);
            medium_function_d(iterations / 3);
            cold_function_c();
            process_data(iterations / 2);
    }
    
    // Always execute these
    analyze_results(iterations / 10);
    conditional_execution(mode);
    
    if (mode == 1 || mode == 2) {
        recursive_function(0, 5);
    }
    
    // Call path selector from lib.c
    path_selector(mode);
    
    printf("Program completed successfully\n");
    return 0;
}
