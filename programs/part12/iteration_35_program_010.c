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
        printf("Negative value: %d\n", value);
    } else if (value == 0) {
        printf("Zero value\n");
    } else {
        printf("Positive value: %d\n", value);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <mode> <value> [iterations]\n", argv[0]);
        return 1;
    }
    
    int mode = atoi(argv[1]);
    int value = atoi(argv[2]);
    int iterations = (argc > 3) ? atoi(argv[3]) : 1000;
    
    // Different execution paths based on mode
    switch (mode) {
        case 1:
            // Path 1: Hot function + lib1
            hot_function(iterations);
            lib1_function_a(value);
            break;
        case 2:
            // Path 2: Cold function + lib2
            cold_function(value);
            lib2_function_a(value);
            break;
        case 3:
            // Path 3: Mix of everything
            hot_function(iterations / 2);
            lib1_function_b(value);
            cold_function(value);
            lib2_function_b(value);
            break;
        case 4:
            // Path 4: Nested conditions
            if (value > 1000) {
                hot_function(iterations * 2);
            } else if (value > 100) {
                lib1_function_a(value);
                lib2_function_a(value);
            } else {
                cold_function(value);
            }
            break;
        default:
            // Default path
            printf("Default mode\n");
            lib1_function_a(value);
            lib2_function_b(value);
    }
    
    // Additional conditional execution
    if (value % 2 == 0) {
        printf("Even value path\n");
    } else {
        printf("Odd value path\n");
    }
    
    return 0;
}
