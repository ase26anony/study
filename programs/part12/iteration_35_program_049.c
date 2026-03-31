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
    
    printf("Input value: %d\n", input_value);
    
    // Vary execution paths based on input
    if (input_value < 10) {
        // Cold path
        cold_function(input_value);
        lib1_function_a();
    } else if (input_value < 100) {
        // Medium path
        hot_function(input_value);
        lib1_function_b();
    } else if (input_value < 1000) {
        // Hot path
        hot_function(input_value * 10);
        lib2_function_a();
    } else {
        // Very hot path
        hot_function(input_value * 100);
        lib2_function_b();
        
        // Nested conditions for more branches
        for (int i = 0; i < (input_value % 100); i++) {
            if (i % 3 == 0) {
                lib1_function_a();
            } else if (i % 3 == 1) {
                lib1_function_b();
            } else {
                lib2_function_a();
            }
        }
    }
    
    // Switch statement for additional branch coverage
    switch (input_value % 5) {
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
        case 4:
            printf("Case 4\n");
            break;
    }
    
    return 0;
}
