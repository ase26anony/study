/* test_loop_doloop.c - Test program for GCC loop-doloop.cc uncovered lines */

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
        sink += i;  /* Non-trivial operation */
        results[0] += i;
    }
    
    /* Loop variant 2: Do-while with pre-decrement */
    j = iterations;
    do {
        sink -= j;  /* Different operation to avoid CSE */
        results[1] += j;
    } while (--j > 0);
    
    /* Loop variant 3: While with post-decrement */
    j = iterations;
    while (j--) {
        sink ^= j;  /* Bitwise operation prevents optimization */
        results[2] += j;
    }
    
    /* Loop variant 4: Nested loops with inner decrement */
    for (i = 3; i > 0; i--) {
        j = iterations / 3;
        while (j-- > 0) {
            sink |= (i * j);  /* Complex enough to keep */
            results[3] += i * j;
        }
    }
    
    /* Use sink to prevent elimination */
    results[4] = sink;
}

/* Another function with different optimization to increase coverage */
__attribute__((noinline, optimize("O1")))
void test_more_loops(int n, int* arr) {
    int i = n;
    
    /* Mixed operations to generate different RTL */
    while (i > 0) {
        arr[i % 10] += i;
        i--;
    }
    
    /* Reverse loop */
    for (i = 0; i < n; i++) {
        arr[i % 10] -= (n - i);
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
    
    /* Non-constant bounds prevent compile-time optimization */
    int results[5] = {0};
    int arr[10] = {0};
    
    /* Call test functions multiple times */
    test_loops(iterations, results);
    test_more_loops(iterations, arr);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 5; i++) {
        checksum += results[i];
    }
    for (int i = 0; i < 10; i++) {
        checksum += arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
