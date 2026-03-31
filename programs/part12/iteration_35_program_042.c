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
        printf("Usage: %s <input_value> [loop_multiplier]\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    int loop_multiplier = (argc > 2) ? atoi(argv[2]) : 1000;
    
    // Branch based on input value
    if (value < 10) {
        printf("Small value path\n");
        lib1_function_a(value);
        cold_function(value);
    } else if (value < 50) {
        printf("Medium value path\n");
        lib1_function_b(value);
        hot_function(loop_multiplier * 10);
    } else if (value < 100) {
        printf("Large value path\n");
        lib2_function_a(value);
        hot_function(loop_multiplier * 100);
    } else {
        printf("Very large value path\n");
        lib2_function_b(value);
        hot_function(loop_multiplier * 1000);
    }
    
    // Switch statement for additional branches
    switch (value % 4) {
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            hot_function(loop_multiplier * 5);
            break;
        case 3:
            printf("Case 3\n");
            cold_function(-value);
            break;
    }
    
    return 0;
}
