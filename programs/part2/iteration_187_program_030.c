#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function declarations from lib.c
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void path_a(void);
void path_b(void);
void mixed_function(int mode);
void utility_function(int x);
void rarely_called(void);
void always_called(void);

// Local functions
void process_data(int size, int mode);
void calculate_stats(int n);
void validate_input(int value);
void recursive_function(int depth);
void loop_intensive(int iterations);

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    // Parse command line arguments to vary execution paths
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Always call these functions
    always_called();
    
    // Vary execution based on mode
    switch (mode) {
        case 1:
            // Mode 1: Heavy on hot paths
            hot_function(iterations * 10);
            process_data(100, 1);
            loop_intensive(iterations);
            mixed_function(1);
            break;
            
        case 2:
            // Mode 2: More balanced
            hot_function(iterations / 2);
            cold_function();
            medium_function(iterations);
            process_data(50, 2);
            mixed_function(2);
            break;
            
        case 3:
            // Mode 3: Different path
            path_a();
            path_b();
            calculate_stats(iterations);
            recursive_function(5);
            mixed_function(3);
            break;
            
        default:
            // Default: Minimal execution
            hot_function(100);
            cold_function();
            break;
    }
    
    // Conditional execution based on iterations
    if (iterations > 500) {
        utility_function(iterations);
        validate_input(iterations);
    }
    
    // Rarely called in some modes
    if (mode == 2 && iterations % 7 == 0) {
        rarely_called();
    }
    
    return 0;
}

void process_data(int size, int mode) {
    int i, j;
    int sum = 0;
    
    // Hot loop
    for (i = 0; i < size; i++) {
        if (mode == 1) {
            // Hot path
            for (j = 0; j < 100; j++) {
                sum += i * j;
            }
        } else if (mode == 2) {
            // Medium path
            sum += i * 2;
        } else {
            // Cold path
            if (i % 10 == 0) {
                sum += i;
            }
        }
    }
    
    // Conditional that's usually true
    if (sum > 0) {
        printf("Processed data: sum = %d\n", sum / 1000);
    } else {
        printf("No data processed\n");
    }
}

void calculate_stats(int n) {
    int i;
    double total = 0.0;
    
    if (n <= 0) {
        printf("Invalid n for stats\n");
        return;
    }
    
    // Loop with varying complexity
    for (i = 1; i <= n; i++) {
        total += 1.0 / i;
        
        // Nested condition
        if (i % 100 == 0) {
            total *= 0.99;
        }
    }
    
    printf("Stats calculated: %f\n", total);
}

void validate_input(int value) {
    // Multiple conditions
    if (value < 0) {
        printf("Negative value\n");
    } else if (value == 0) {
        printf("Zero value\n");
    } else if (value > 1000) {
        printf("Large value: %d\n", value);
    } else {
        printf("Normal value\n");
    }
    
    // Switch statement
    switch (value % 4) {
        case 0:
            printf("Divisible by 4\n");
            break;
        case 1:
            printf("Remainder 1\n");
            break;
        case 2:
            printf("Remainder 2\n");
            break;
        default:
            printf("Remainder 3\n");
            break;
    }
}

void recursive_function(int depth) {
    if (depth <= 0) {
        return;
    }
    
    printf("Recursion depth: %d\n", depth);
    
    // Recursive call with condition
    if (depth % 2 == 0) {
        recursive_function(depth - 1);
    } else {
        recursive_function(depth - 2);
    }
}

void loop_intensive(int iterations) {
    int i, j, k;
    long counter = 0;
    
    // Triple nested loop - very hot
    for (i = 0; i < iterations / 10; i++) {
        for (j = 0; j < 10; j++) {
            for (k = 0; k < 10; k++) {
                counter += i * j * k;
            }
        }
        
        // Conditional inside hot loop
        if (i % 100 == 0) {
            printf("Progress: %d\n", i);
        }
    }
    
    printf("Loop intensive completed: %ld\n", counter);
}
