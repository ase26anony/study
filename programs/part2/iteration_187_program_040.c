#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Forward declarations for functions in lib.c
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void rarely_called(void);
void varying_function(int mode);

// Functions in main.c
void process_data(int size, int mode) {
    int i;
    if (mode == 1) {
        // Hot path - executed many times
        for (i = 0; i < size * 100; i++) {
            if (i % 7 == 0) {
                printf("Processing data point %d\n", i);
            }
        }
    } else if (mode == 2) {
        // Medium path
        for (i = 0; i < size * 10; i++) {
            if (i % 3 == 0) {
                printf("Alternative processing %d\n", i);
            }
        }
    } else {
        // Cold path
        printf("Minimal processing for mode %d\n", mode);
    }
}

void analyze_results(int depth) {
    if (depth <= 0) return;
    
    // Recursive function with varying depth
    printf("Analyzing at depth %d\n", depth);
    analyze_results(depth - 1);
    
    // Conditional branch
    if (depth % 2 == 0) {
        printf("Even depth analysis\n");
    } else {
        printf("Odd depth analysis\n");
    }
}

void handle_input(char* input) {
    int i = 0;
    int sum = 0;
    
    // Process input string
    while (input[i] != '\0') {
        if (input[i] >= '0' && input[i] <= '9') {
            sum += input[i] - '0';
        }
        i++;
    }
    
    printf("Input sum: %d\n", sum);
    
    // Nested condition
    if (sum > 20) {
        printf("High sum detected\n");
    } else if (sum > 10) {
        printf("Medium sum detected\n");
    } else {
        printf("Low sum detected\n");
    }
}

void complex_logic(int a, int b, int c) {
    // Multiple branches for coverage
    if (a > b) {
        printf("a > b\n");
        if (b > c) {
            printf("Descending order\n");
        }
    } else if (a < b) {
        printf("a < b\n");
        if (b < c) {
            printf("Ascending order\n");
        }
    } else {
        printf("a == b\n");
    }
    
    // Switch statement
    switch (a % 4) {
        case 0:
            printf("Multiple of 4\n");
            break;
        case 1:
            printf("One more than multiple of 4\n");
            break;
        case 2:
            printf("Two more than multiple of 4\n");
            break;
        default:
            printf("Three more than multiple of 4\n");
    }
}

int main(int argc, char* argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    // Parse command line arguments
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Seed random number generator
    srand(time(NULL));
    
    // Call functions with different frequencies based on mode
    if (mode == 1) {
        // Mode 1: Heavy execution
        for (int i = 0; i < iterations; i++) {
            hot_function(100);
            if (i % 10 == 0) {
                medium_function(50);
            }
            if (i % 100 == 0) {
                cold_function();
            }
        }
        process_data(10, 1);
        analyze_results(5);
    } else if (mode == 2) {
        // Mode 2: Medium execution
        for (int i = 0; i < iterations / 10; i++) {
            hot_function(20);
            if (i % 5 == 0) {
                medium_function(10);
            }
        }
        process_data(5, 2);
        analyze_results(3);
        rarely_called();
    } else if (mode == 3) {
        // Mode 3: Light execution
        hot_function(5);
        medium_function(2);
        cold_function();
        rarely_called();
        process_data(1, 3);
        analyze_results(1);
    } else {
        // Default mode: Mixed execution
        hot_function(iterations / 100);
        medium_function(iterations / 200);
        cold_function();
        rarely_called();
        varying_function(mode);
        process_data(3, mode);
        analyze_results(2);
    }
    
    // Additional function calls
    handle_input("Test123");
    complex_logic(mode, iterations % 10, rand() % 20);
    
    printf("Program completed successfully\n");
    return 0;
}
