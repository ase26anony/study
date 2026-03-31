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
        printf("Negative path\n");
    } else if (value == 0) {
        printf("Zero path\n");
    } else if (value < 100) {
        printf("Small positive path\n");
    } else {
        printf("Large positive path\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_value>\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    
    // Branch based on input value
    switch (value % 4) {
        case 0:
            hot_function(1000000);  // Very hot
            lib1_function_a(value);
            break;
        case 1:
            hot_function(500000);   // Moderately hot
            lib1_function_b(value);
            break;
        case 2:
            hot_function(1000);     // Warm
            lib2_function_a(value);
            break;
        case 3:
            hot_function(10);       // Cold
            lib2_function_b(value);
            break;
    }
    
    // More conditional logic
    if (value > 1000) {
        cold_function(value);
        for (int i = 0; i < 100; i++) {
            // Another hot block
            volatile int y = i * i;
        }
    } else if (value > 100) {
        cold_function(-value);
    } else {
        cold_function(0);
    }
    
    // Nested conditions
    if (value % 2 == 0) {
        if (value % 3 == 0) {
            printf("Divisible by 6\n");
        } else {
            printf("Even but not divisible by 3\n");
        }
    }
    
    return 0;
}
