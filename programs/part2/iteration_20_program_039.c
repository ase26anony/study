/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical loops */
#define NOINLINE_NOOPT __attribute__((noinline, optimize("O1")))

/* Volatile variables to prevent dead code elimination */
static volatile int global_sum = 0;
static volatile int global_counter = 0;

/* External array to prevent constant propagation */
extern int external_array[];

/* Function containing the loops we want to test */
NOINLINE_NOOPT
void test_loops(int iterations, int* results) {
    int i, j, k, m;
    int local_sum = 0;
    
    /* Loop Variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        /* Non-trivial operation with side effect */
        results[i % 256] = i;
        local_sum += external_array[i % 256];
        global_counter++;
    }
    results[0] = local_sum;
    
    /* Loop Variant 2: do-while with decrement */
    j = iterations;
    local_sum = 0;
    do {
        results[j % 256] = j * 2;
        local_sum -= external_array[j % 256];
        global_counter++;
    } while (--j > 0);
    results[1] = local_sum;
    
    /* Loop Variant 3: while loop with post-decrement */
    k = iterations;
    local_sum = 0;
    while (k--) {
        results[k % 256] = k * 3;
        local_sum ^= external_array[k % 256];
        global_counter++;
    }
    results[2] = local_sum;
    
    /* Loop Variant 4: Nested loops with inner decrementing counter */
    m = iterations / 10;
    local_sum = 0;
    for (i = 0; i < 10; i++) {
        int inner = m;
        while (inner > 0) {
            results[(i * m + inner) % 256] = i * inner;
            local_sum += external_array[(i * m + inner) % 256] * i;
            global_counter++;
            inner--;
        }
    }
    results[3] = local_sum;
    
    /* Store final result to volatile to ensure execution */
    global_sum = results[0] + results[1] + results[2] + results[3];
}

/* External array definition - prevents constant propagation */
int external_array[256];

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 100) {
        fprintf(stderr, "Iterations must be >= 100\n");
        return 1;
    }
    
    /* Initialize external array with non-constant values */
    for (int i = 0; i < 256; i++) {
        external_array[i] = (i * 37 + 123) % 7919;  /* Prime number pattern */
    }
    
    /* Results array on heap to prevent stack optimization */
    int* results = (int*)malloc(256 * sizeof(int));
    if (!results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize results array */
    for (int i = 0; i < 256; i++) {
        results[i] = 0;
    }
    
    /* Call the function with our loops */
    test_loops(iterations, results);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d, %d, %d, %d\n", 
           results[0], results[1], results[2], results[3]);
    printf("Global sum: %d\n", global_sum);
    printf("Global counter: %d\n", global_counter);
    
    free(results);
    return 0;
}
