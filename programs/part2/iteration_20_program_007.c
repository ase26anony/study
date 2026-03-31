/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of the test function */
__attribute__((noinline, optimize("O1")))
void test_loops(int iterations, int* results) {
    volatile int sink = 0;  /* Volatile to prevent elimination */
    int i, j, k;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        sink += i;  /* Side effect to prevent loop removal */
        results[0] += sink;
    }
    
    /* Loop variant 2: do-while with pre-decrement */
    j = iterations;
    do {
        sink -= j;
        results[1] += sink;
    } while (--j > 0);
    
    /* Loop variant 3: while with post-decrement */
    k = iterations;
    while (k--) {
        sink *= 2;
        results[2] += sink;
    }
    
    /* Loop variant 4: Nested loops with inner decrement */
    for (i = 10; i > 0; i--) {
        int inner = iterations / 10;
        while (inner-- > 0) {
            sink ^= inner;
            results[3] += sink;
        }
    }
    
    /* Final side effect */
    results[4] = sink;
}

/* Another test function with different optimization attributes */
#pragma GCC push_options
#pragma GCC optimize("O1")
__attribute__((noinline))
void test_more_loops(int n, int* arr) {
    int i, j;
    
    /* Loop with array access - harder to optimize away */
    for (i = n; i > 0; i--) {
        arr[i % 100] = i;
    }
    
    /* Complex exit condition */
    j = n;
    while (1) {
        arr[j % 100] += j;
        if (--j <= 0) break;
    }
}
#pragma GCC pop_options

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
    
    /* Use volatile to prevent constant propagation */
    volatile int vol_iterations = iterations;
    
    int results[5] = {0};
    int array[100] = {0};
    
    /* Call test functions multiple times */
    test_loops(vol_iterations, results);
    test_more_loops(vol_iterations, array);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += results[i];
    }
    for (int i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
