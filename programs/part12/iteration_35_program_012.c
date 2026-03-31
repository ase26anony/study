#include <stdio.h>
#include <stdlib.h>
#include "module1.h"
#include "module2.h"

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
    int input_value = 0;
    
    if (argc > 1) {
        input_value = atoi(argv[1]);
    } else {
        FILE *f = fopen("input.txt", "r");
        if (f) {
            fscanf(f, "%d", &input_value);
            fclose(f);
        }
    }
    
    // Create varied execution paths based on input
    switch (input_value % 4) {
        case 0:
            hot_function(1000000);  // Very hot
            module1_func_a();
            break;
        case 1:
            hot_function(500000);   // Moderately hot
            module1_func_b();
            cold_function(input_value);
            break;
        case 2:
            hot_function(10000);    // Less hot
            module2_func_a();
            break;
        case 3:
            hot_function(100);      // Cold
            module2_func_b();
            cold_function(-input_value);
            break;
    }
    
    // Additional conditional logic
    if (input_value > 1000) {
        for (int i = 0; i < input_value / 100; i++) {
            // Another hot block for high values
            volatile int y = i % 7;
        }
    } else if (input_value < -100) {
        printf("Very negative input\n");
    }
    
    return 0;
}
