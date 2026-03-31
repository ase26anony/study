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
    int input_value = 0;
    
    if (argc > 1) {
        input_value = atoi(argv[1]);
    } else {
        // Read from file if no command line argument
        FILE *f = fopen("input.txt", "r");
        if (f) {
            fscanf(f, "%d", &input_value);
            fclose(f);
        }
    }
    
    // Varying execution paths based on input
    switch (input_value % 4) {
        case 0:
            hot_function(1000000);  // Very hot
            lib1_function_a();
            break;
        case 1:
            hot_function(500000);   // Moderately hot
            lib1_function_b();
            lib2_function_x();
            break;
        case 2:
            hot_function(10000);    // Less hot
            cold_function(input_value);
            lib2_function_y();
            break;
        case 3:
            hot_function(1000);     // Cold
            cold_function(-1);      // Rare path
            lib1_function_a();
            lib2_function_x();
            break;
    }
    
    // Additional conditional logic
    if (input_value > 1000) {
        for (int i = 0; i < input_value / 100; i++) {
            // Variable hotness based on input
            volatile int y = i % 7;
        }
    }
    
    return 0;
}
