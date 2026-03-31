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
    volatile int sink = 0;  /* Volatile to prevent dead code elimination */
    int i, j;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        sink += i;  /* Non-trivial operation */
        results[0] += sink;
    }
    
    /* Loop variant 2: Do-while loop */
    j = iterations;
    do {
        sink -= j;
        results[1] ^= sink;  /* Different operation to avoid merging */
        j--;
    } while (j > 0);
    
    /* Loop variant 3: While loop with post-decrement */
    int k = iterations;
    while (k--) {
        sink *= 2;
        results[2] += sink % 256;
    }
    
    /* Loop variant 4: Nested loops */
    int m = iterations / 2;
    for (i = 0; i < 3; i++) {
        int n = m;
        while (n-- > 0) {
            sink += i * n;
            results[3] += sink;
        }
    }
    
    /* Final volatile store to ensure all loops execute */
    results[4] = sink;
}

/* Another function with different optimization level to test more patterns */
__attribute__((noinline))
#pragma GCC optimize("O1")
void more_loops(int n, int* arr) {
    /* Loop with pointer decrement */
    int* ptr = arr + n;
    while (ptr > arr) {
        *--ptr = n;
        n--;
    }
    
    /* Loop with explicit compare against 0 */
    for (int i = n; i != 0; i = i - 1) {
        arr[i] = i * 2;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 10) {
        fprintf(stderr, "Iterations must be >= 10\n");
        return 1;
    }
    
    /* Use dynamic allocation to prevent constant propagation */
    int* results = (int*)malloc(5 * sizeof(int));
    memset(results, 0, 5 * sizeof(int));
    
    /* Call the test function multiple times to increase coverage chance */
    test_loops(iterations, results);
    more_loops(iterations / 2, results);
    
    /* Print results to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += results[i];
        printf("Result[%d] = %d\n", i, results[i]);
    }
    printf("Total sum: %d\n", sum);
    
    free(results);
    return 0;
}
