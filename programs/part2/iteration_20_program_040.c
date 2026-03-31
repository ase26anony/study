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
    int i, j, k;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        sink += i;  /* Side effect to prevent removal */
        results[0] += sink;
    }
    
    /* Loop variant 2: Do-while with pre-decrement */
    j = iterations;
    do {
        sink -= j;
        results[1] += sink;
    } while (--j > 0);
    
    /* Loop variant 3: While with post-decrement */
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
}

/* Another function with different optimization to increase coverage */
__attribute__((noinline, optimize("O2")))
void test_more_loops(int n, int* arr) {
    volatile int counter = 0;
    
    /* Loop with compound condition */
    for (int i = n; i > 0; i -= 1) {
        arr[i % 100] = i;
        counter += arr[i % 100];
    }
    
    /* Reverse counting loop */
    int j = n;
    while (j) {
        arr[j % 50] += j;
        counter -= arr[j % 50];
        j--;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 20) {
        printf("Please use at least 20 iterations\n");
        return 1;
    }
    
    /* Non-constant bounds to prevent constant propagation */
    volatile int dynamic_iterations = iterations;
    
    int results[4] = {0};
    int array[100] = {0};
    
    /* Call both test functions */
    test_loops(dynamic_iterations, results);
    test_more_loops(dynamic_iterations, array);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += results[i];
    }
    for (int i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    printf("Result checksum: %d\n", sum);
    return 0;
}
