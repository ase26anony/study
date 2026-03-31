#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function declarations from lib.c
extern void process_data(int iterations, int mode);
extern void analyze_results(int threshold);
extern void generate_report(int detail_level);
extern void validate_input(int value);
extern void cleanup_resources(void);

// Local functions
void hot_function_1(int iterations) {
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            printf("Even iteration: %d\n", i);
        } else {
            printf("Odd iteration: %d\n", i);
        }
    }
}

void hot_function_2(int limit) {
    int sum = 0;
    for (int i = 0; i < limit; i++) {
        sum += i;
        if (i % 10 == 0) {
            printf("Sum at %d: %d\n", i, sum);
        }
    }
    printf("Final sum: %d\n", sum);
}

void cold_function_1(void) {
    printf("Cold function 1 executed\n");
    // Rarely executed code
    for (int i = 0; i < 3; i++) {
        printf("  Cold loop iteration %d\n", i);
    }
}

void cold_function_2(int value) {
    if (value > 1000) {
        printf("Value exceeds threshold: %d\n", value);
    } else {
        printf("Value within normal range: %d\n", value);
    }
}

void medium_function(int base) {
    int result = base;
    for (int i = 0; i < 50; i++) {
        result += i * 2;
        if (result % 7 == 0) {
            printf("Divisible by 7 at i=%d\n", i);
        }
    }
    printf("Medium function result: %d\n", result);
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
    
    // Vary execution based on mode
    switch (mode) {
        case 1:
            // Mode 1: Heavy execution of hot functions
            hot_function_1(iterations * 2);
            hot_function_2(iterations);
            process_data(iterations, 1);
            analyze_results(50);
            break;
            
        case 2:
            // Mode 2: Mix of hot and cold functions
            hot_function_1(iterations / 2);
            cold_function_1();
            process_data(iterations, 2);
            generate_report(1);
            break;
            
        case 3:
            // Mode 3: More cold paths
            cold_function_2(iterations);
            medium_function(iterations);
            validate_input(iterations);
            generate_report(2);
            break;
            
        default:
            // Default: Balanced execution
            hot_function_1(iterations);
            medium_function(iterations);
            process_data(iterations, 0);
            analyze_results(30);
            generate_report(0);
            break;
    }
    
    // Always execute cleanup
    cleanup_resources();
    
    return 0;
}
