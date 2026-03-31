/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of loops */
#define NO_OPT __attribute__((optimize("O1")))

/* Volatile variables to prevent dead code elimination */
volatile int global_sum = 0;
volatile int sink;

/* External function to create side effects */
void __attribute__((noinline, noclone)) side_effect(int x) {
    sink = x;
}

/* Function containing the loops we want to test */
NO_OPT void test_loops(int iterations, int* array) {
    int i, j;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        array[i % 256] = i * 2;
        side_effect(array[i % 256]);
    }
    
    /* Loop variant 2: Do-while with pre-decrement */
    j = iterations;
    do {
        array[j % 256] = j * 3;
        side_effect(array[j % 256]);
    } while (--j > 0);
    
    /* Loop variant 3: While loop with post-decrement */
    j = iterations;
    while (j--) {
        array[j % 256] = j * 4;
        side_effect(array[j % 256]);
        global_sum += array[j % 256];
    }
    
    /* Loop variant 4: Nested loops with inner decrementing counter */
    for (i = 0; i < 10; i++) {
        int inner = iterations / 10;
        while (inner-- > 0) {
            array[(i * inner) % 256] = i * inner;
            side_effect(array[(i * inner) % 256]);
        }
    }
    
    /* Loop variant 5: Complex decrement pattern */
    for (i = iterations; i != 0; i -= 1) {
        if (i % 2 == 0) {
            array[i % 256] = i * 5;
        } else {
            array[i % 256] = i * 7;
        }
        side_effect(array[i % 256]);
    }
}

/* Main function with command line argument */
int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 20) {
        fprintf(stderr, "Iterations must be >= 20\n");
        return 1;
    }
    
    /* Non-constant array to prevent optimization */
    int* array = (int*)malloc(256 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 256; i++) {
        array[i] = i * iterations;
    }
    
    /* Call the function with loops */
    test_loops(iterations, array);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += array[i];
    }
    
    printf("Result: array sum = %d, global_sum = %d\n", sum, global_sum);
    
    free(array);
    return 0;
}
