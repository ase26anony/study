#include <stdio.h>
#include <stdlib.h>
#include "lib1.h"
#include "lib2.h"

void hot_function(int iterations) {
    for (int i = 0; i < iterations; i++) {
        // Hot block - executed many times
        volatile int x = i * 2;
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
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_value> <loop_iterations>\n", argv[0]);
        return 1;
    }
    
    int input = atoi(argv[1]);
    int iterations = atoi(argv[2]);
    
    // Vary execution paths based on input
    switch (input % 4) {
        case 0:
            lib1_function_a(iterations);
            hot_function(iterations / 2);
            break;
        case 1:
            lib1_function_b(iterations);
            cold_function(input);
            break;
        case 2:
            lib2_function_a(iterations);
            hot_function(iterations);
            break;
        case 3:
            lib2_function_b(iterations);
            cold_function(-input);
            break;
    }
    
    // Additional conditional logic
    if (input > 1000) {
        // Very hot path for large inputs
        for (int i = 0; i < iterations * 10; i++) {
            volatile int y = i * 3;
        }
    } else if (input < 0) {
        // Very cold path
        printf("Special negative case\n");
    }
    
    return 0;
}
