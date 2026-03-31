#include <stdio.h>
#include <stdlib.h>

// Function with high execution count potential
void hot_function_1(int iterations) {
    for (int i = 0; i < iterations; i++) {
        // Hot block - will be executed many times
        volatile int x = i * 2;
    }
}

// Function with conditional execution
void conditional_function_1(int value) {
    if (value > 100) {
        printf("Value is large: %d\n", value);
    } else if (value > 50) {
        printf("Value is medium: %d\n", value);
    } else {
        printf("Value is small: %d\n", value);
    }
}

// Another function for object-level analysis
void utility_function_1(void) {
    // Simple utility
    static int counter = 0;
    counter++;
}
