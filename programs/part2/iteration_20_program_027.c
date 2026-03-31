/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of critical loops */
#define NOINLINE_NOOPT __attribute__((noinline, optimize("O1")))

/* Volatile variables to prevent dead code elimination */
volatile int volatile_sink = 0;
volatile int volatile_source = 42;

/* External function to create side effects */
NOINLINE_NOOPT
static void side_effect(int x) {
    volatile_sink = x;
}

/* Array with external linkage to prevent constant propagation */
int global_array[1024];

/* Main test function with loops designed to generate (reg + -1) COMPARE 0 pattern */
NOINLINE_NOOPT
static void test_loops(int iterations, int* results) {
    int i, j, k, m;
    int sum = 0;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        /* Non-trivial operation with side effect */
        results[i % 1024] = volatile_source + i;
        side_effect(results[i % 1024]);
    }
    
    /* Loop variant 2: do-while with pre-decrement */
    j = iterations;
    if (j > 0) {
        do {
            results[j % 1024] += j;
            side_effect(results[j % 1024]);
        } while (--j > 0);
    }
    
    /* Loop variant 3: while with post-decrement */
    k = iterations;
    while (k--) {
        results[k % 1024] *= 2;
        side_effect(results[k % 1024]);
    }
    
    /* Loop variant 4: Nested loops with inner decrementing counter */
    for (m = 0; m < 10; m++) {
        int inner = iterations / 10;
        while (inner-- > 0) {
            results[(m * inner) % 1024] -= m;
            side_effect(results[(m * inner) % 1024]);
        }
    }
    
    /* Loop variant 5: Complex decrement with computation in condition */
    int n = iterations;
    int temp;
    while ((temp = n) > 0) {
        results[temp % 1024] = temp * 3;
        side_effect(results[temp % 1024]);
        n = temp - 1;
    }
}

/* Wrapper to ensure loops aren't optimized away */
NOINLINE_NOOPT
static int compute_result(int iterations) {
    int local_results[1024];
    int final_sum = 0;
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 1024; i++) {
        local_results[i] = i + volatile_source;
    }
    
    /* Run the test loops */
    test_loops(iterations, local_results);
    
    /* Compute a checksum to prevent dead code elimination */
    for (int i = 0; i < 1024; i++) {
        final_sum += local_results[i];
    }
    
    return final_sum;
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
    
    /* Ensure we have enough iterations for meaningful loops */
    if (iterations > 1000000) {
        iterations = 1000000;
    }
    
    printf("Running with %d iterations...\n", iterations);
    
    int result = compute_result(iterations);
    
    printf("Result checksum: %d\n", result);
    printf("Volatile sink: %d\n", volatile_sink);
    
    return 0;
}
