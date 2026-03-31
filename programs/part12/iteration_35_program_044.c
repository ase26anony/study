#include <stdio.h>
#include <stdlib.h>
#include "lib1.h"
#include "lib2.h"

void hot_function(int iterations) {
    for (int i = 0; i < iterations; i++) {
        // Hot block - executed many times
        volatile int x = i * i;
    }
}

void cold_function(int value) {
    if (value < 0) {
        // Cold block - rarely executed
        printf("Negative value: %d\n", value);
    } else if (value == 0) {
        // Another cold block
        printf("Zero value\n");
    } else {
        // Hotter block
        printf("Positive value: %d\n", value);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_value> [loop_iterations]\n", argv[0]);
        return 1;
    }
    
    int input = atoi(argv[1]);
    int iterations = (argc > 2) ? atoi(argv[2]) : 1000;
    
    // Vary execution paths based on input
    if (input < 10) {
        // Path 1: Call lib1 functions
        lib1_function_a(input);
        lib1_function_b(input * 2);
    } else if (input < 50) {
        // Path 2: Call lib2 functions with hot loop
        hot_function(iterations);
        lib2_function_a(input);
    } else if (input < 100) {
        // Path 3: Mix of functions
        lib1_function_a(input);
        lib2_function_b(input);
        cold_function(input);
    } else {
        // Path 4: All functions with very hot loop
        hot_function(iterations * 10);
        lib1_function_b(input);
        lib2_function_a(input);
        cold_function(-input);  // Rare negative path
    }
    
    // Complex branching for coverage
    switch (input % 5) {
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            hot_function(100);
            break;
        case 3:
            printf("Case 3\n");
            cold_function(input);
            break;
        case 4:
            printf("Case 4\n");
            // Nested conditions
            if (input % 3 == 0) {
                lib1_function_a(input);
            } else {
                lib2_function_b(input);
            }
            break;
    }
    
    return 0;
}
