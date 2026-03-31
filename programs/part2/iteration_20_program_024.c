/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of loops */
#define NOOPT __attribute__((optimize("O1")))

/* Volatile variable to prevent dead code elimination */
static volatile int sink = 0;

/* External function to prevent inlining */
extern void external_side_effect(int) __attribute__((noinline, noclone));
void external_side_effect(int x) {
    sink += x;
}

/* Array with non-constant access pattern */
static int array[1024];

/* Function containing the loops - marked with specific optimization level */
NOOPT void test_loops(int iterations, int* results) {
    int i, j, k;
    int n = iterations;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        /* Non-trivial operation with side effect */
        array[i % 1024] = i;
        external_side_effect(i);
    }
    
    /* Loop variant 2: do-while loop */
    j = n;
    if (j > 0) {
        do {
            array[j % 1024] += j;
            external_side_effect(j);
        } while (--j > 0);
    }
    
    /* Loop variant 3: while loop with post-decrement */
    k = n;
    while (k--) {
        array[k % 1024] *= 2;
        external_side_effect(k);
    }
    
    /* Loop variant 4: Nested loops with inner decrementing counter */
    for (i = 0; i < 10; i++) {
        int inner = n / 10;
        while (inner-- > 0) {
            array[(i * inner) % 1024] += inner;
            external_side_effect(inner);
        }
    }
    
    /* Store results to prevent elimination */
    results[0] = array[n % 1024];
    results[1] = sink;
}

/* Alternative version with different optimization pragma */
#pragma GCC push_options
#pragma GCC optimize ("O1")
void test_loops2(int iterations, int* results) {
    int n = iterations;
    int sum = 0;
    
    /* Another decrementing loop variant */
    for (int i = n; i != 0; i--) {
        /* Use volatile memory operation */
        *(volatile int*)&array[i % 1024] = i;
        sum += i;
    }
    
    results[2] = sum;
    results[3] = array[0];
}
#pragma GCC pop_options

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 20) {
        fprintf(stderr, "Iterations should be >= 20 for meaningful test\n");
        iterations = 100;
    }
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 1024; i++) {
        array[i] = i + 1;
    }
    
    int results[4] = {0};
    
    /* Call the test functions */
    test_loops(iterations, results);
    test_loops2(iterations, results);
    
    /* Print results to prevent elimination */
    printf("Results: %d %d %d %d\n", 
           results[0], results[1], results[2], results[3]);
    
    return 0;
}
