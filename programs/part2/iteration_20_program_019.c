/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical functions */
#define NOINLINE_NOOPT __attribute__((noinline, optimize("O1")))

/* Volatile variables to prevent dead code elimination */
static volatile int volatile_sink = 0;
static volatile int* volatile_ptr = &volatile_sink;

/* External function to create side effects */
NOINLINE_NOOPT
static void do_work(int value) {
    *volatile_ptr = value;
}

/* Array to work on - passed as parameter to prevent constant propagation */
NOINLINE_NOOPT
static void test_loops(int iterations, int* array) {
    int i, j;
    int n = iterations;
    
    /* Loop Variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        array[i % 256] = i;
        do_work(i);
    }
    
    /* Reset counter */
    n = iterations;
    
    /* Loop Variant 2: do-while loop */
    do {
        array[n % 256] = n;
        do_work(n);
    } while (--n > 0);
    
    /* Reset counter */
    n = iterations;
    
    /* Loop Variant 3: while loop with post-decrement */
    while (n--) {
        array[n % 256] = n;
        do_work(n);
    }
    
    /* Loop Variant 4: Nested loops with inner decrementing loop */
    n = iterations / 2;
    for (i = 0; i < 5; i++) {
        int inner = n;
        /* Inner loop with decrementing counter */
        for (j = inner; j > 0; j--) {
            array[(i * j) % 256] = i * j;
            do_work(i * j);
        }
    }
    
    /* Loop Variant 5: Complex decrement pattern */
    n = iterations;
    while (1) {
        array[n % 256] = n;
        do_work(n);
        if (--n == 0) break;
    }
}

/* Main function with command line argument */
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
    
    /* Use stack array to avoid heap allocation complexity */
    int array[256];
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        array[i] = 0;
    }
    
    /* Call the test function */
    test_loops(iterations, array);
    
    /* Compute and print result to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += array[i];
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
