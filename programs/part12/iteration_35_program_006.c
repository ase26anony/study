#include <stdio.h>
#include <stdlib.h>
#include "module1.h"
#include "module2.h"

void hot_function(int iterations) {
    for (int i = 0; i < iterations; i++) {
        // Hot block - executed many times
        volatile int x = i * 2;
    }
}

void cold_function(int value) {
    if (value < 0) {
        printf("Negative value\n");
    } else if (value == 0) {
        printf("Zero value\n");
    } else {
        printf("Positive value\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_value>\n", argv[0]);
        return 1;
    }
    
    int input = atoi(argv[1]);
    
    // Create varied execution paths based on input
    if (input > 1000) {
        // Hot path - large loop
        hot_function(input);
        module1_high_frequency();
    } else if (input > 500) {
        // Medium path
        module1_medium_frequency();
        cold_function(input);
    } else if (input > 100) {
        // Low frequency path
        module2_low_frequency();
    } else if (input > 0) {
        // Rare path
        module2_rare_function();
    } else {
        // Very rare path
        printf("Special case\n");
    }
    
    // Always execute some common code
    common_function();
    
    return 0;
}
