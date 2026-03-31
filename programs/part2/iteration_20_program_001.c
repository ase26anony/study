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
static volatile int volatile_sink;
static volatile int* volatile_ptr;

/* External function to create side effects */
NOINLINE_NOOPT
static void side_effect(int value) {
    volatile_sink = value;
}

/* Main test function with multiple loop variants */
NOINLINE_NOOPT
static int test_loops(int iterations, int* array) {
    int i, j;
    int result = 0;
    
    /* Variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        array[i % 256] = i;           /* Non-constant array index */
        side_effect(array[i % 256]);  /* Force side effect */
        result += array[i % 256];
    }
    
    /* Variant 2: Do-while loop with decrement */
    j = iterations;
    do {
        array[j % 256] = j * 2;
        side_effect(array[j % 256]);
        result += array[j % 256];
    } while (--j > 0);
    
    /* Variant 3: While loop with post-decrement */
    int k = iterations;
    while (k--) {
        array[k % 256] = k * 3;
        side_effect(array[k % 256]);
        result += array[k % 256];
    }
    
    /* Variant 4: Nested loops with inner decrementing counter */
    int outer;
    for (outer = 10; outer > 0; outer--) {
        int inner = iterations / 10;
        while (inner-- > 0) {
            array[(outer + inner) % 256] = outer + inner;
            side_effect(array[(outer + inner) % 256]);
            result += array[(outer + inner) % 256];
        }
    }
    
    return result;
}

/* Alternative test with different optimization pragma */
#pragma GCC push_options
#pragma GCC optimize ("O1")
NOINLINE_NOOPT
static int test_loops_alt(int iterations, int* array) {
    int m = iterations;
    int sum = 0;
    
    /* Another variant: while with pre-decrement */
    while (1) {
        array[m % 256] = m * 4;
        side_effect(array[m % 256]);
        sum += array[m % 256];
        if (--m <= 0) break;
    }
    
    return sum;
}
#pragma GCC pop_options

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iteration_count>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 100) {
        fprintf(stderr, "Iteration count must be >= 100\n");
        return 1;
    }
    
    /* Non-constant sized array to prevent optimization */
    int* array = (int*)malloc(256 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 256; i++) {
        array[i] = i * i;
    }
    
    volatile_ptr = array;  /* Mark array as volatile through pointer */
    
    int result1 = test_loops(iterations, array);
    int result2 = test_loops_alt(iterations / 2, array);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Final: %d\n", result1 + result2 + array[iterations % 256]);
    
    free(array);
    return 0;
}
