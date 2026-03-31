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
    volatile int sink = 0;  /* Volatile to prevent elimination */
    int i, j, k;
    int n = iterations;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        sink += i;  /* Side effect prevents removal */
        results[0] += sink;
    }
    
    /* Loop variant 2: Do-while with pre-decrement */
    j = n;
    do {
        sink -= j;
        results[1] += sink;
    } while (--j > 0);
    
    /* Loop variant 3: While with post-decrement */
    k = n;
    while (k--) {
        sink *= 2;
        results[2] += sink;
    }
    
    /* Loop variant 4: Nested loops with decrementing inner counter */
    for (i = 0; i < 5; i++) {
        int inner = n / 2;
        while (inner-- > 0) {
            sink ^= inner;
            results[3] += sink;
        }
    }
    
    /* Force use of sink to prevent dead code elimination */
    results[4] = sink;
}

/* Another function with different optimization level to test pattern */
__attribute__((noinline, optimize("O1")))
void test_more_loops(int iterations, int* arr, int size) {
    int i, n = iterations;
    
    /* Loop with array access - harder to optimize away */
    for (i = n; i > 0; i--) {
        arr[i % size] += i;
    }
    
    /* Another decrementing loop */
    while (n-- > 0) {
        arr[n % size] ^= n;
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
    
    /* Non-constant bounds from command line */
    int results[5] = {0};
    int* arr = (int*)malloc(iterations * sizeof(int));
    
    if (!arr) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < iterations; i++) {
        arr[i] = i * 3;
    }
    
    /* Test both functions */
    test_loops(iterations, results);
    test_more_loops(iterations, arr, iterations);
    
    /* Print results to prevent elimination */
    printf("Results: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", results[i]);
    }
    printf("\nArray sum: %d\n", arr[iterations / 2]);
    
    free(arr);
    return 0;
}
