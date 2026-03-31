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
        sink += i * 2;
        results[0] += sink;
    }
    
    /* Loop variant 2: Do-while with decrement */
    j = iterations;
    do {
        sink -= j;
        results[1] ^= sink;
        j--;
    } while (j > 0);
    
    /* Loop variant 3: While loop with post-decrement */
    int k = iterations;
    while (k--) {
        sink += k * 3;
        results[2] |= sink;
    }
    
    /* Loop variant 4: Nested loops with inner decrement */
    int outer = iterations / 10;
    for (int o = 0; o < outer; o++) {
        int inner = 10;
        while (inner--) {
            sink += o * inner;
            results[3] += sink;
        }
    }
    
    /* Loop variant 5: Complex decrement pattern */
    int m = iterations;
    while (m > 0) {
        sink = sink * 2 + m;
        results[4] = sink;
        m -= 1;  /* Explicit decrement by 1 */
    }
}

/* Another function with different optimization to increase coverage */
__attribute__((noinline, optimize("O1")))
void more_loops(int n, int* arr) {
    volatile int acc = 0;
    
    /* Reverse counting loop */
    for (int cnt = n; cnt != 0; cnt = cnt - 1) {
        arr[cnt % 16] += acc;
        acc = cnt * 3;
    }
    
    /* Loop with if inside */
    int x = n;
    while (x) {
        if (x & 1) {
            acc += x;
        }
        x--;
    }
    arr[0] = acc;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 20) {
        fprintf(stderr, "Need at least 20 iterations\n");
        return 1;
    }
    
    /* Non-constant bounds prevent compile-time optimization */
    int results[5] = {0};
    int arr[16] = {0};
    
    test_loops(iterations, results);
    more_loops(iterations / 2, arr);
    
    /* Use results to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += results[i];
    }
    for (int i = 0; i < 16; i++) {
        sum += arr[i];
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
