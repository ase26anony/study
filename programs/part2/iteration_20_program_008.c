/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of critical function */
__attribute__((noinline, optimize("O1")))
void test_loops(int iterations, int* results) {
    volatile int sink = 0;  /* Prevent dead code elimination */
    int i, j;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        sink += i;
        results[0] += i;
    }
    
    /* Loop variant 2: Do-while with pre-decrement */
    j = iterations;
    do {
        sink -= j;
        results[1] -= j;
    } while (--j > 0);
    
    /* Loop variant 3: While loop with post-decrement */
    j = iterations;
    while (j--) {
        sink ^= j;
        results[2] ^= j;
    }
    
    /* Loop variant 4: Nested loops with inner decrementing counter */
    for (i = 10; i > 0; i--) {
        j = iterations / 10;
        while (j-- > 0) {
            sink |= i + j;
            results[3] |= i + j;
        }
    }
    
    /* Force use of sink to prevent optimization */
    results[4] = sink;
}

/* Another function with different optimization to increase chances */
__attribute__((noinline, optimize("O1")))
void more_loops(int n, int* arr) {
    int i;
    
    /* Loop with array access - harder to optimize away */
    for (i = n; i > 0; i--) {
        arr[i % 100] = i;
    }
    
    /* Loop with pointer decrement */
    int* ptr = &arr[99];
    while (ptr >= &arr[0]) {
        *ptr = n--;
        ptr--;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 10) {
        printf("Iterations must be >= 10\n");
        return 1;
    }
    
    /* Use volatile to prevent constant propagation */
    volatile int vol_iter = iterations;
    
    int results[5] = {0};
    int arr[100] = {0};
    
    /* Call test functions multiple times */
    test_loops(vol_iter, results);
    more_loops(vol_iter, arr);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += results[i];
    }
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
