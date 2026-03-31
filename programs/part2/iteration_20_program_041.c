/* test_loop_doloop.c - Test program for GCC loop-doloop.cc pattern matching */
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
        sink += i;  /* Non-removable side effect */
        results[0] += sink;
    }
    
    /* Loop variant 2: do-while with pre-decrement */
    j = iterations;
    do {
        sink -= j;
        results[1] += sink;
    } while (--j > 0);
    
    /* Loop variant 3: while with post-decrement */
    j = iterations;
    while (j--) {
        sink ^= j;
        results[2] ^= sink;
    }
    
    /* Loop variant 4: Nested loops with inner decrement */
    for (i = 3; i > 0; i--) {
        j = iterations / 3;
        while (j-- > 0) {
            sink = sink * 2 + 1;
            results[3] += sink;
        }
    }
    
    /* Force use of results */
    results[4] = sink;
}

/* Another function with different optimization level to test pattern */
__attribute__((noinline, optimize("O1")))
void test_more_loops(int n, int* arr) {
    int i = n;
    
    /* Complex decrement pattern */
    while (i > 0) {
        arr[i % 16] += i;
        i--;
    }
    
    /* Another pattern */
    for (i = n; i != 0; i--) {
        arr[(i * 7) % 16] ^= i;
    }
}

int main(int argc, char** argv) {
    int iterations;
    int results[5] = {0};
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    iterations = atoi(argv[1]);
    if (iterations < 10) {
        fprintf(stderr, "Iterations must be >= 10\n");
        return 1;
    }
    
    /* Test with different iteration counts */
    test_loops(iterations, results);
    
    int arr[16] = {0};
    test_more_loops(iterations / 2, arr);
    
    /* Print results to prevent optimization */
    printf("Results: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", results[i]);
    }
    printf("\nArray sum: ");
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += arr[i];
    }
    printf("%d\n", sum);
    
    return 0;
}
