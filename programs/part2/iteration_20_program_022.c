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
    
    /* Loop 1: Basic for loop with decrementing counter */
    for (i = iterations; i > 0; i--) {
        sink += i;  /* Non-trivial operation */
        results[0] += sink;
    }
    
    /* Loop 2: do-while with pre-decrement */
    j = iterations;
    do {
        sink += j;
        results[1] ^= sink;  /* Different operation to avoid merging */
        __asm__ volatile ("" : : "r"(sink) : "memory");  /* Memory barrier */
    } while (--j > 0);
    
    /* Loop 3: while with post-decrement */
    k = iterations;
    while (k--) {
        sink -= k;  /* Different computation */
        results[2] |= sink;
        __asm__ volatile ("" : : "r"(results[2]) : "memory");
    }
    
    /* Loop 4: Nested loops - inner loop uses decrementing counter */
    int outer = iterations / 10;
    for (i = 0; i < outer; i++) {
        int inner = 10;
        while (inner--) {
            sink += i * inner;
            results[3] += sink;
        }
    }
    
    /* Loop 5: Complex decrement pattern with if condition */
    int m = iterations;
    while (m > 0) {
        if (m & 1) {
            sink += m;
        } else {
            sink -= m;
        }
        results[4] += sink;
        m--;
    }
}

/* Another function with different optimization attributes */
__attribute__((noinline, optimize("O2")))
void test_more_loops(int iterations, int* results) {
    volatile int sink = 0;
    
    /* Loop with pointer arithmetic */
    int* ptr = results + 5;
    int count = iterations;
    while (count-- > 0) {
        *ptr++ = sink++;
    }
    
    /* Loop with explicit comparison */
    for (int n = iterations; n != 0; n = n - 1) {
        sink = sink ^ n;
        results[6] += sink;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 20) {
        printf("Iterations should be >= 20 for meaningful test\n");
        iterations = 100;
    }
    
    /* Non-constant array to prevent optimization */
    int* results = (int*)malloc(10 * sizeof(int));
    memset(results, 0, 10 * sizeof(int));
    
    /* Call test functions multiple times with different values */
    test_loops(iterations, results);
    test_more_loops(iterations / 2, results);
    
    /* Compute and print checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum ^= results[i];
    }
    printf("Result checksum: %d\n", checksum);
    
    free(results);
    return 0;
}
