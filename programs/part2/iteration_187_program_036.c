#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function declarations from lib.c
extern void lib_func1(int iterations);
extern void lib_func2(int iterations);
extern void lib_func3(int iterations);
extern void lib_func4(int iterations);
extern int lib_hot_function(int iterations);
extern int lib_cold_function(int iterations);

// Local functions with varying execution frequencies
void hot_function(int iterations) {
    for (int i = 0; i < iterations; i++) {
        // Hot loop - executed many times
        int x = i * 2;
        if (x % 3 == 0) {
            x += 1;
        }
    }
}

void warm_function(int iterations) {
    for (int i = 0; i < iterations / 2; i++) {
        // Warm loop - executed moderately
        int y = i * 3;
        if (y % 2 == 0) {
            y -= 1;
        }
    }
}

void cold_function(int iterations) {
    if (iterations > 1000) {
        // Rarely executed branch
        for (int i = 0; i < 10; i++) {
            int z = i * 5;
        }
    } else {
        // More common branch
        int z = iterations * 2;
    }
}

void mixed_function(int mode) {
    switch (mode) {
        case 1:
            // Frequently executed in mode 1
            for (int i = 0; i < 100; i++) {
                int a = i * i;
            }
            break;
        case 2:
            // Frequently executed in mode 2
            for (int i = 0; i < 50; i++) {
                int b = i * 2;
                if (b > 25) {
                    b = 25;
                }
            }
            break;
        default:
            // Rarely executed
            int c = mode * 10;
            break;
    }
}

void recursive_function(int depth, int max_depth) {
    if (depth >= max_depth) {
        return;
    }
    
    // Create some work
    int local = depth * 2;
    if (local % 3 == 0) {
        recursive_function(depth + 1, max_depth);
    } else {
        recursive_function(depth + 2, max_depth);
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    // Parse command line arguments to vary execution
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Vary execution based on mode to create different profiles
    switch (mode) {
        case 1:
            // Mode 1: Focus on hot functions
            hot_function(iterations * 10);
            warm_function(iterations * 5);
            lib_func1(iterations * 8);
            lib_hot_function(iterations * 12);
            break;
            
        case 2:
            // Mode 2: Different execution pattern
            hot_function(iterations * 5);
            cold_function(iterations);
            lib_func2(iterations * 3);
            lib_cold_function(iterations);
            break;
            
        case 3:
            // Mode 3: Mixed execution
            mixed_function(1);
            mixed_function(2);
            mixed_function(3);
            lib_func3(iterations * 2);
            lib_func4(iterations);
            break;
            
        default:
            // Default mode: All functions
            hot_function(iterations);
            warm_function(iterations / 2);
            cold_function(iterations / 10);
            mixed_function(mode % 3);
            recursive_function(0, 5);
            lib_func1(iterations);
            lib_func2(iterations / 2);
            lib_func3(iterations / 3);
            lib_func4(iterations / 4);
            lib_hot_function(iterations * 2);
            lib_cold_function(iterations / 5);
            break;
    }
    
    // Additional conditional execution based on iterations
    if (iterations > 5000) {
        // Rarely executed path
        for (int i = 0; i < 100; i++) {
            int rare = i * i * i;
        }
    } else if (iterations > 1000) {
        // Sometimes executed
        for (int i = 0; i < 50; i++) {
            int common = i * 2;
        }
    }
    
    return 0;
}
