/* test_loops.c - Test program for GCC loop-doloop pattern matching */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of critical function */
__attribute__((noinline, optimize("O1")))
void test_loops(int iterations, int* results) {
    volatile int sink = 0;  /* Prevent dead code elimination */
    int i, j;
    
    /* Loop 1: Basic for loop with decrementing counter */
    for (i = iterations; i > 0; i--) {
        sink += i;  /* Non-trivial operation */
        results[0] += sink;
    }
    
    /* Loop 2: do-while with pre-decrement */
    j = iterations;
    do {
        sink -= j;
        results[1] += sink;
    } while (--j > 0);
    
    /* Loop 3: while loop with post-decrement */
    int k = iterations;
    while (k--) {
        sink *= 2;
        results[2] += sink;
    }
    
    /* Loop 4: Nested loops with inner decrementing counter */
    int m = iterations / 2;
    for (i = 0; i < 3; i++) {
        int inner = m;
        while (inner-- > 0) {
            sink ^= inner;
            results[3] += sink;
        }
    }
    
    /* Loop 5: Complex decrement pattern */
    int n = iterations;
    for (; n > 0; n -= 1) {
        if (sink > 1000) sink = 0;
        results[4] += n;
    }
}

/* Another function with different optimization to increase coverage */
__attribute__((noinline, optimize("O1")))
void more_loops(int n, int* arr) {
    volatile int acc = 0;
    
    /* Loop with array access - harder to optimize away */
    for (int i = n; i > 0; i--) {
        arr[i % 10] += i;
        acc += arr[i % 10];
    }
    
    /* Reverse counting loop */
    int j = n;
    while (j) {
        arr[j % 5] -= j;
        acc -= arr[j % 5];
        j--;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 10) {
        fprintf(stderr, "Iterations must be > 10\n");
        return 1;
    }
    
    /* Non-constant bounds from command line */
    int results[5] = {0};
    int array[10] = {0};
    
    /* Call test functions */
    test_loops(iterations, results);
    more_loops(iterations, array);
    
    /* Print results to prevent optimization */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += results[i];
    }
    for (int i = 0; i < 10; i++) {
        total += array[i];
    }
    
    printf("Result: %d\n", total);
    return 0;
}
