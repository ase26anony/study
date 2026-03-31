/* test_loops.c - Test program for GCC loop-doloop.cc pattern matching */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of individual loops */
#define NOOPT __attribute__((optimize("O1")))

/* Volatile variables to prevent dead code elimination */
volatile int global_sum = 0;
volatile int global_counter = 0;

/* External function to create side effects */
void __attribute__((noinline, noclone)) side_effect(int x) {
    global_sum += x;
}

/* Function containing the loops we want to test */
NOOPT void test_loops(int iterations, int* array) {
    int i, j;
    
    /* Loop 1: Basic for loop with decrementing counter */
    for (i = iterations; i > 0; i--) {
        /* Non-trivial operation with side effect */
        array[i % 256] = i;
        side_effect(i);
    }
    
    /* Loop 2: do-while loop with decrement */
    j = iterations;
    do {
        array[j % 256] = j * 2;
        side_effect(j);
    } while (--j > 0);
    
    /* Loop 3: while loop with post-decrement */
    int k = iterations;
    while (k--) {
        array[k % 256] = k * 3;
        side_effect(k);
    }
    
    /* Loop 4: Nested loops with inner decrementing counter */
    int outer = iterations / 10;
    for (int m = 0; m < outer; m++) {
        int inner = 10;
        while (inner--) {
            array[(m * 10 + inner) % 256] = m + inner;
            side_effect(m + inner);
        }
    }
    
    /* Loop 5: Another variant with explicit comparison */
    int n = iterations;
    while (n > 0) {
        array[n % 256] = n * 4;
        side_effect(n);
        n--;
    }
}

/* Main function with command line argument */
int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 100) {
        fprintf(stderr, "Iterations should be >= 100 for meaningful test\n");
        iterations = 100;
    }
    
    /* Non-constant array to prevent optimization */
    int* array = (int*)malloc(256 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3 + iterations;
    }
    
    /* Call the test function multiple times */
    for (int run = 0; run < 3; run++) {
        test_loops(iterations + run, array);
    }
    
    /* Compute and print result to prevent dead code elimination */
    int result = 0;
    for (int i = 0; i < 256; i++) {
        result += array[i];
    }
    
    printf("Result: %d, Global sum: %d\n", result, global_sum);
    
    free(array);
    return 0;
}
