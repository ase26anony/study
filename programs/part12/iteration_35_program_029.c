#include <stdio.h>
#include <stdlib.h>

// Function with high execution count (hot)
void hot_function1(int iterations) {
    for (int i = 0; i < iterations; i++) {
        // Hot block - executed many times
        volatile int x = i * 2;
    }
}

// Function with medium execution count
void medium_function(int value) {
    if (value > 100) {
        printf("High value: %d\n", value);
    } else if (value > 50) {
        printf("Medium value: %d\n", value);
    } else {
        printf("Low value: %d\n", value);
    }
}

// Rarely called function (cold)
void cold_function1(void) {
    printf("This function is rarely called\n");
}
