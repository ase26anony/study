/* test_loops.c - Test program for GCC loop-doloop.cc pattern matching */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of the test function */
__attribute__((noinline, optimize("O1")))
void test_loops(int iterations, int* results) {
    volatile int sink = 0;  /* Prevent dead code elimination */
    int i, j, k;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        sink += i;  /* Non-removable side effect */
        results[0] += i;
    }
    
    /* Loop variant 2: Do-while with decrement */
    j = iterations;
    do {
        sink -= j;  /* Different operation to avoid CSE */
        results[1] += j;
    } while (--j > 0);
    
    /* Loop variant 3: While loop with post-decrement */
    k = iterations;
    while (k--) {
        sink ^= k;  /* Another different operation */
        results[2] += k;
    }
    
    /* Loop variant 4: Nested loops with inner decrement */
    for (i = 0; i < 5; i++) {
        int inner = iterations / 10;
        while (inner-- > 0) {
            sink |= inner;  /* Bitwise operation */
            results[3] += inner + i;
        }
    }
    
    /* Ensure sink is used */
    results[4] = sink;
}

/* Another function with different optimization level to test */
__attribute__((noinline, optimize("O1")))
void test_more_loops(int n, int* arr) {
    int i, sum = 0;
    
    /* Loop with array access - harder to optimize away */
    for (i = n; i > 0; i--) {
        arr[i % 100] = i;  /* Non-constant array index */
        sum += arr[(i + 1) % 100];
    }
    
    /* Store result to prevent elimination */
    arr[0] = sum;
}

int main(int argc, char** argv) {
    int iterations;
    int results[5] = {0};
    int array[100] = {0};
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    iterations = atoi(argv[1]);
    if (iterations < 10) {
        fprintf(stderr, "Iterations must be >= 10\n");
        return 1;
    }
    
    /* Call test functions multiple times with different values */
    test_loops(iterations, results);
    test_more_loops(iterations, array);
    
    /* Use results to prevent dead code elimination */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += results[i];
    }
    total += array[0];
    
    printf("Result: %d\n", total);
    return 0;
}
