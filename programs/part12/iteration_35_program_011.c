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
    } else {
        printf("Positive path\n");
    }
}

int main(int argc, char *argv[]) {
    int input = 0;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    } else {
        // Read from file if no command line argument
        FILE *f = fopen("input.txt", "r");
        if (f) {
            fscanf(f, "%d", &input);
            fclose(f);
        }
    }
    
    printf("Input: %d\n", input);
    
    // Create varied execution paths based on input
    switch (input % 4) {
        case 0:
            hot_function(1000000);  // Very hot
            lib1_function_a();
            break;
        case 1:
            hot_function(500000);   // Moderately hot
            lib1_function_b();
            cold_function(input);
            break;
        case 2:
            hot_function(10000);    // Warm
            lib2_function_a();
            break;
        case 3:
            // Cold path - minimal execution
            cold_function(input);
            lib2_function_b();
            break;
    }
    
    // Additional conditional logic
    if (input > 1000) {
        printf("Large input detected\n");
        for (int i = 0; i < 100; i++) {
            // Another hot block for large inputs
            volatile int y = i * input;
        }
    } else if (input < -100) {
        printf("Very negative input\n");
    }
    
    return 0;
}
