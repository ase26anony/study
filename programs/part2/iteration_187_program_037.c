#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib.h"

#define HOT_LOOP_COUNT 100000
#define COLD_LOOP_COUNT 100

/* Hot function - executed many times */
void hot_function_1(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i % 100;
        if (i % 1000 == 0) {
            sum += i;  // Rarely executed branch
        }
    }
    printf("Hot function 1 result: %d\n", sum % 1000);
}

/* Cold function - executed rarely */
void cold_function_1(void) {
    printf("Cold function 1 executed\n");
    for (int i = 0; i < 10; i++) {
        if (i % 3 == 0) {
            printf("  Multiple of 3: %d\n", i);
        }
    }
}

/* Medium frequency function */
void medium_function(int mode) {
    static int counter = 0;
    counter++;
    
    if (mode == 1) {
        for (int i = 0; i < 1000; i++) {
            if (i % 100 == 0) {
                printf("Medium: %d\n", i);
            }
        }
    } else {
        printf("Medium function alternate path\n");
    }
}

/* Function with complex branching */
void branching_function(int value) {
    if (value < 0) {
        printf("Negative value: %d\n", value);
    } else if (value < 10) {
        printf("Small positive: %d\n", value);
    } else if (value < 100) {
        printf("Medium positive: %d\n", value);
    } else {
        printf("Large positive: %d\n", value);
    }
    
    switch (value % 4) {
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            break;
        default:
            printf("Case default\n");
            break;
    }
}

/* Recursive function */
int recursive_function(int n, int depth) {
    if (depth <= 0 || n <= 0) {
        return 1;
    }
    
    if (n % 2 == 0) {
        return n + recursive_function(n - 1, depth - 1);
    } else {
        return n * recursive_function(n - 2, depth - 1);
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = HOT_LOOP_COUNT;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode %d\n", mode);
    
    /* Vary execution based on mode to create different profiles */
    if (mode == 1) {
        // Mode 1: Heavy execution of hot functions
        for (int i = 0; i < 10; i++) {
            hot_function_1(iterations);
            lib_hot_function(iterations / 10);
        }
        
        // Execute cold functions rarely
        if (mode % 10 == 0) {
            cold_function_1();
        }
        
        // Call library functions
        lib_complex_function(50, 3);
        lib_utility_function(100);
        
    } else if (mode == 2) {
        // Mode 2: More balanced execution
        for (int i = 0; i < 5; i++) {
            hot_function_1(iterations / 2);
            lib_hot_function(iterations / 20);
        }
        
        // Execute cold functions more often
        cold_function_1();
        lib_cold_function();
        
        // More medium functions
        for (int i = 0; i < 20; i++) {
            medium_function(i % 2);
        }
        
        // Call library functions
        lib_complex_function(30, 5);
        lib_utility_function(200);
        
    } else {
        // Mode 3: Different execution pattern
        hot_function_1(iterations / 10);
        
        // Lots of branching
        for (int i = -5; i < 15; i++) {
            branching_function(i);
        }
        
        // Recursive calls
        int result = recursive_function(10, 5);
        printf("Recursive result: %d\n", result);
        
        // Library calls
        lib_utility_function(50);
        lib_cold_function();
    }
    
    // Always execute some common code
    medium_function(mode);
    branching_function(mode * 10);
    
    // Call library function that's always executed
    lib_common_function();
    
    printf("Program completed in mode %d\n", mode);
    return 0;
}
