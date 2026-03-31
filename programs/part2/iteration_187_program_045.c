#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib.h"

/* Hot functions (executed many times) */
void hot_function_1(int iterations) {
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
}

void hot_function_2(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i * i;
        if (sum > 1000) {
            sum = 0;
        }
    }
    printf("Sum of squares: %d\n", sum);
}

/* Cold functions (executed rarely) */
void cold_function_1(void) {
    printf("Cold function 1 executed\n");
    // Rarely executed path
    if (rand() % 1000 == 0) {
        printf("Lucky path!\n");
    }
}

void cold_function_2(int value) {
    if (value < 0) {
        printf("Negative value\n");
    } else if (value == 0) {
        printf("Zero value\n");
    } else {
        printf("Positive value\n");
    }
}

/* Medium frequency function */
void medium_function(int mode) {
    switch (mode) {
        case 1:
            printf("Mode 1\n");
            break;
        case 2:
            printf("Mode 2\n");
            break;
        case 3:
            printf("Mode 3\n");
            break;
        default:
            printf("Unknown mode\n");
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 100;
    
    srand(time(NULL));
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Vary execution based on mode to create different profiles
    switch (mode) {
        case 1:
            // Mode 1: Heavy on hot_function_1
            hot_function_1(iterations * 2);
            hot_function_2(iterations / 2);
            cold_function_1();
            lib_function_a(iterations);
            lib_function_c(10);
            break;
            
        case 2:
            // Mode 2: Heavy on hot_function_2 and lib functions
            hot_function_1(iterations / 4);
            hot_function_2(iterations * 3);
            cold_function_2(mode);
            lib_function_b(iterations);
            lib_function_d(50);
            break;
            
        case 3:
            // Mode 3: Balanced execution
            for (int i = 0; i < 3; i++) {
                hot_function_1(iterations / 3);
                hot_function_2(iterations / 3);
                medium_function(i);
            }
            lib_function_a(iterations / 2);
            lib_function_b(iterations / 2);
            break;
            
        default:
            // Default: Minimal execution
            hot_function_1(10);
            cold_function_2(mode);
            lib_function_c(5);
    }
    
    // Call lib functions with varying frequencies
    if (mode % 2 == 0) {
        lib_rarely_called();
    }
    
    return 0;
}
