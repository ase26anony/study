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
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_value> <loop_count>\n", argv[0]);
        return 1;
    }
    
    int input = atoi(argv[1]);
    int loop_count = atoi(argv[2]);
    
    // Vary execution paths based on input
    if (input < 10) {
        lib1_function_a(input);
        cold_function(input);
    } else if (input < 50) {
        lib1_function_b(input);
        hot_function(loop_count / 2);
    } else if (input < 100) {
        lib2_function_a(input);
        hot_function(loop_count);
    } else {
        lib2_function_b(input);
        hot_function(loop_count * 2);
    }
    
    // Additional conditional execution
    switch (input % 4) {
        case 0:
            printf("Case 0\n");
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
