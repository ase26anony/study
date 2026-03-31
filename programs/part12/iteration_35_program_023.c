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
    
    // Branch coverage based on input
    if (input < 10) {
        printf("Small input\n");
        lib1_function_a(input);
    } else if (input < 100) {
        printf("Medium input\n");
        lib1_function_b(input);
        hot_function(1000);  // Create hot block
    } else if (input < 1000) {
        printf("Large input\n");
        lib2_function_a(input);
        hot_function(10000); // Very hot block
    } else {
        printf("Very large input\n");
        lib2_function_b(input);
        hot_function(100000); // Extremely hot block
    }
    
    // More conditional execution
    switch (input % 4) {
        case 0:
            cold_function(input);
            break;
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            break;
        case 3:
            printf("Case 3\n");
            break;
    }
    
    return 0;
}
