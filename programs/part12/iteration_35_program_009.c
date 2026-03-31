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
    if (argc < 2) {
        printf("Usage: %s <input_value>\n", argv[0]);
        return 1;
    }
    
    int input = atoi(argv[1]);
    
    // Control execution paths based on input
    if (input > 1000) {
        // Very hot path
        hot_function(input);
        lib1_hot(input / 10);
    } else if (input > 100) {
        // Medium path
        hot_function(input / 2);
        lib1_cold(input);
        lib2_process(input);
    } else if (input > 10) {
        // Cold path
        cold_function(input);
        lib2_rare(input);
    } else {
        // Very cold path
        printf("Default case\n");
        lib1_default();
        lib2_default();
    }
    
    // Additional branching for coverage
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
