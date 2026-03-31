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
        printf("Negative value: %d\n", value);
    } else if (value == 0) {
        printf("Zero value\n");
    } else {
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
    
    // Varying execution paths based on input
    switch (input % 4) {
        case 0:
            hot_function(iterations * 10);  // Very hot
            lib1_function_a(input);
            break;
        case 1:
            hot_function(iterations);       // Moderately hot
            lib1_function_b(input);
            break;
        case 2:
            cold_function(input);           // Cold
            lib2_function_a(input);
            break;
        case 3:
            hot_function(iterations / 2);   // Warm
            lib2_function_b(input);
            break;
    }
    
    // Additional conditional logic
    if (input > 1000) {
        for (int i = 0; i < input % 100; i++) {
            // Another hot block for high inputs
            volatile int y = i * 3;
        }
    } else if (input < 0) {
        printf("Processing negative input\n");
    }
    
    return 0;
}
