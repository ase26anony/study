/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of loops */
#define NO_OPT __attribute__((optimize("O1")))

/* Volatile variables to prevent dead code elimination */
static volatile int global_sum = 0;
static volatile int global_counter = 0;

/* Non-inlineable function to create side effects */
NO_OPT static void side_effect(int value) {
    global_sum += value;
    global_counter++;
}

/* Function containing the loops we want to test */
NO_OPT static void test_loops(int iterations, int* array) {
    int i, j;
    
    /* Loop Variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        array[i % 256] = i;
        side_effect(i);
    }
    
    /* Loop Variant 2: do-while with pre-decrement */
    j = iterations;
    if (j > 0) {
        do {
            array[j % 256] = j * 2;
            side_effect(j);
        } while (--j > 0);
    }
    
    /* Loop Variant 3: while loop with post-decrement */
    j = iterations;
    while (j--) {
        array[j % 256] = j * 3;
        side_effect(j);
    }
    
    /* Loop Variant 4: Nested loops with inner decrementing counter */
    for (i = 10; i > 0; i--) {
        int inner = iterations % 100;
        while (inner-- > 0) {
            array[(i + inner) % 256] = i * inner;
            side_effect(i + inner);
        }
    }
    
    /* Loop Variant 5: Complex decrement pattern */
    for (i = iterations; i != 0; i -= 1) {
        array[i % 256] = i * i;
        side_effect(i * i);
    }
}

/* Alternative version with different optimization pragma */
#pragma GCC push_options
#pragma GCC optimize ("O1")
NO_OPT static void test_loops_alt(int iterations, int* array) {
    int n = iterations;
    
    /* Another variant: while with compound condition */
    while (n > 0) {
        array[n % 256] = n;
        side_effect(n);
        n--;
    }
    
    /* Reverse counting with decrement */
    for (int k = iterations; k > 0; --k) {
        array[k % 256] = k * k;
        side_effect(k);
    }
}
#pragma GCC pop_options

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 100) {
        fprintf(stderr, "Iterations should be >= 100 for meaningful test\n");
        iterations = 100;
    }
    
    /* Non-constant array to prevent optimization */
    int* array = (int*)malloc(256 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 256; i++) {
        array[i] = (i * 13) % 97;
    }
    
    /* Reset global counters */
    global_sum = 0;
    global_counter = 0;
    
    /* Test the loops */
    test_loops(iterations, array);
    test_loops_alt(iterations, array);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += array[i];
    }
    
    printf("Array sum: %d\n", sum);
    printf("Global sum: %d\n", global_sum);
    printf("Global counter: %d\n", global_counter);
    
    free(array);
    return 0;
}
