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
    int n = iterations;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        sink += i;  /* Side effect to prevent removal */
        results[0] += i;
    }
    
    /* Loop variant 2: do-while with pre-decrement */
    j = n;
    do {
        sink -= j;
        results[1] += j;
    } while (--j > 0);
    
    /* Loop variant 3: while with post-decrement */
    k = n;
    while (k--) {
        sink *= 2;
        results[2] += k + 1;
    }
    
    /* Loop variant 4: Nested loops with inner decrement */
    int outer = n / 10;
    for (i = 0; i < outer; i++) {
        int inner = 10;
        while (inner-- > 0) {
            sink ^= inner;
            results[3] += inner;
        }
    }
    
    /* Force use of sink to prevent dead code elimination */
    results[4] = sink;
}

/* Another function with different optimization to increase chances */
__attribute__((noinline, optimize("O1")))
void more_loops(int n, int* arr) {
    volatile int acc = 0;
    
    /* Loop with decrement by 1, testing against 0 */
    for (int i = n; i != 0; i--) {
        arr[i % 100] = i;
        acc += arr[i % 100];
    }
    
    /* Another pattern: while with decrement */
    while (n-- > 0) {
        acc ^= n;
        arr[n % 100] = acc;
    }
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
    
    /* Use non-constant, non-static arrays to prevent optimization */
    int* results1 = (int*)malloc(5 * sizeof(int));
    int* results2 = (int*)malloc(100 * sizeof(int));
    
    if (!results1 || !results2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    memset(results1, 0, 5 * sizeof(int));
    memset(results2, 0, 100 * sizeof(int));
    
    /* Call test functions */
    test_loops(iterations, results1);
    more_loops(iterations, results2);
    
    /* Compute and print result to prevent elimination */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += results1[i];
    }
    for (int i = 0; i < 100; i++) {
        total += results2[i];
    }
    
    printf("Result: %d\n", total);
    
    free(results1);
    free(results2);
    
    return 0;
}
