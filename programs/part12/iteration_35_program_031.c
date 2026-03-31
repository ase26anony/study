#include <stdio.h>
#include <stdlib.h>
#include "lib1.h"
#include "lib2.h"

void hot_function(int iterations) {
    for (int i = 0; i < iterations; i++) {
        // Hot block - high execution count
        volatile int x = i * 2;
    }
}

void cold_function(int value) {
    if (value < 0) {
        // Cold block - rarely executed
        printf("Negative value: %d\n", value);
    } else if (value == 0) {
        // Another cold path
        printf("Zero value\n");
    } else {
        // Hotter path
        printf("Positive value: %d\n", value);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_value> <loop_iterations>\n", argv[0]);
        return 1;
    }
    
    int input_value = atoi(argv[1]);
    int iterations = atoi(argv[2]);
    
    // Vary execution paths based on input
    if (input_value < 10) {
        lib1_function_a(input_value);
        cold_function(input_value);
    } else if (input_value < 50) {
        lib1_function_b(input_value);
        hot_function(iterations / 10);
    } else if (input_value < 100) {
        lib2_function_a(input_value);
        hot_function(iterations);
    } else {
        lib2_function_b(input_value);
        if (input_value % 2 == 0) {
            hot_function(iterations * 2);
        } else {
            cold_function(-input_value);
        }
    }
    
    // Always execute some common code
    common_function();
    
    return 0;
}
