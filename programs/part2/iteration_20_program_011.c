/* test_loops.c - Test program for GCC loop-doloop.cc pattern matching */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of critical function */
__attribute__((noinline, optimize("O1")))
void test_loops(int iterations, int* results) {
    volatile int sink = 0;  /* Volatile to prevent elimination */
    int i, j;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        sink += i;  /* Side effect */
        results[0] += sink;
    }
    
    /* Loop variant 2: Do-while with pre-decrement */
    j = iterations;
    do {
        sink -= j;  /* Different side effect */
        results[1] += sink;
    } while (--j > 0);
    
    /* Loop variant 3: While loop with post-decrement */
    j = iterations;
    while (j--) {
        sink ^= j;  /* Another side effect */
        results[2] += sink;
    }
    
    /* Loop variant 4: Nested loops with inner decrement */
    for (i = 2; i > 0; i--) {
        j = iterations;
        while (j > 0) {
            sink |= j;  /* Bitwise operation */
            results[3] += sink;
            j--;
        }
    }
    
    /* Loop variant 5: Complex decrement pattern */
    j = iterations;
    while (1) {
        sink = sink * 3 + 1;
        results[4] += sink;
        if (--j == 0) break;
    }
}

/* Alternative function with different optimization pragma */
#pragma GCC push_options
#pragma GCC optimize("O1")
__attribute__((noinline))
void test_more_loops(int iterations, int* results) {
    volatile int counter = iterations;
    int sum = 0;
    
    /* Loop with volatile counter */
    while (counter > 0) {
        sum += counter;
        results[5] = sum;
        counter--;
    }
    
    /* Loop with array access */
    int arr[100];
    for (int i = 0; i < 100; i++) arr[i] = i;
    
    for (int i = iterations; i > 0; i--) {
        arr[i % 100] += sum;
        results[6] += arr[i % 100];
    }
}
#pragma GCC pop_options

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
    
    /* Non-constant bounds to prevent constant propagation */
    int results[10] = {0};
    
    /* Call test functions multiple times */
    test_loops(iterations, results);
    test_more_loops(iterations, results);
    
    /* Compute final result to prevent dead code elimination */
    int final_result = 0;
    for (int i = 0; i < 10; i++) {
        final_result += results[i];
    }
    
    printf("Result: %d\n", final_result);
    return 0;
}
