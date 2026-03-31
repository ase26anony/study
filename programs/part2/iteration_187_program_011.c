#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declarations for functions in lib.c
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void rarely_called(void);
void path_a(void);
void path_b(void);
void mixed_function(int mode);

// Functions defined in main.c
void process_data(int mode, int iterations) {
    if (mode == 0) {
        // Hot path - executed many times
        for (int i = 0; i < iterations; i++) {
            hot_function(i);
        }
    } else if (mode == 1) {
        // Medium path
        medium_function(iterations / 2);
    } else {
        // Cold path
        cold_function();
        rarely_called();
    }
}

void analyze_results(int count) {
    int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += i;
        if (i % 100 == 0) {
            mixed_function(i % 3);
        }
    }
    
    if (sum > 1000) {
        path_a();
    } else {
        path_b();
    }
}

void initialize_system(void) {
    printf("Initializing system...\n");
    // Some initialization logic
    for (int i = 0; i < 10; i++) {
        // Do some work
    }
}

void cleanup_system(void) {
    printf("Cleaning up...\n");
    // Cleanup logic
}

int main(int argc, char *argv[]) {
    int mode = 0;
    int iterations = 1000;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    initialize_system();
    
    // Vary execution based on mode
    switch (mode) {
        case 0:
            // Mode 0: Heavy execution
            process_data(0, iterations);
            analyze_results(iterations * 2);
            break;
        case 1:
            // Mode 1: Medium execution
            process_data(1, iterations / 2);
            analyze_results(iterations);
            break;
        case 2:
            // Mode 2: Light execution
            process_data(2, 1);
            analyze_results(10);
            break;
        default:
            // Default: Mixed execution
            for (int i = 0; i < 3; i++) {
                process_data(i % 3, iterations / (i + 1));
            }
            break;
    }
    
    cleanup_system();
    return 0;
}
