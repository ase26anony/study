/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop_doloop
 * Or with RISC-V target: gcc -O1 -march=rv64gc -fdump-rtl-expand test_loop_doloop.c -o test_loop_doloop
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of individual loops */
#define NO_OPT __attribute__((optimize("O1")))

/* External function to prevent inlining and create side effects */
void __attribute__((noinline, noclone)) side_effect(int *p, int val) {
    *p += val;
}

/* Volatile variable to prevent dead code elimination */
static volatile int volatile_sink = 0;

/* Function containing the loops we want to test */
NO_OPT void test_loops(int iterations, int *array) {
    int i, j;
    
    /* Loop Variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        /* Non-trivial operation that can't be optimized away */
        array[i % 256] = i;
        volatile_sink += i;
    }
    
    /* Loop Variant 2: do-while loop with decrement */
    j = iterations;
    if (j > 0) {
        do {
            /* Different operation to avoid CSE */
            array[(j * 3) % 256] ^= j;
            volatile_sink -= j;
            j--;
        } while (j > 0);
    }
    
    /* Loop Variant 3: while loop with post-decrement */
    int k = iterations;
    while (k--) {
        /* Use external function call for side effect */
        side_effect(&array[k % 256], k);
        volatile_sink |= k;
    }
    
    /* Loop Variant 4: Nested loops with inner decrementing counter */
    int outer = iterations / 10;
    for (int m = 0; m < outer; m++) {
        int inner = 10;
        while (inner--) {
            /* Complex enough to not be optimized away */
            array[(m * 10 + inner) % 256] += m * inner;
            volatile_sink = volatile_sink * 2 + 1;
        }
    }
    
    /* Loop Variant 5: Another for loop with different computation */
    for (int n = iterations; n > 0; n -= 1) {
        /* Use memory operation with volatile-like behavior */
        int idx = n % 256;
        array[idx] = array[idx] * 3 + n;
        volatile_sink = volatile_sink ^ array[idx];
    }
}

/* Main function with command-line argument */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 100) {
        fprintf(stderr, "Iterations should be at least 100\n");
        return 1;
    }
    
    /* Use heap allocation to prevent stack optimization */
    int *array = (int *)malloc(256 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 256; i++) {
        array[i] = i + 1;
    }
    
    /* Call the test function */
    test_loops(iterations, array);
    
    /* Compute and print a result to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += array[i];
    }
    
    printf("Result: sum=%d, volatile_sink=%d\n", sum, volatile_sink);
    
    free(array);
    return 0;
}
