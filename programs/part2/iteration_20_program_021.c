/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of loops */
#define NOOPT __attribute__((optimize("O1")))

/* Volatile variables to prevent dead code elimination */
volatile int g_result = 0;
volatile int g_side_effect = 0;

/* External function to create side effects */
NOOPT void __attribute__((noinline, noclone)) 
do_work(int *arr, int idx, int val) {
    arr[idx] = val;
    g_side_effect += val;
}

/* Function containing the loops we want to test */
NOOPT void __attribute__((noinline, noclone))
test_loops(int iterations, int *array) {
    int i, j;
    
    /* Loop 1: Basic for loop with decrementing counter */
    for (i = iterations; i > 0; i--) {
        do_work(array, i % 100, i);
    }
    
    /* Loop 2: do-while loop with decrement */
    j = iterations;
    do {
        do_work(array, j % 100, j);
    } while (--j > 0);
    
    /* Loop 3: while loop with post-decrement */
    int k = iterations;
    while (k--) {
        do_work(array, k % 100, k);
    }
    
    /* Loop 4: Nested loops with inner decrementing counter */
    int outer = iterations / 10;
    for (int o = 0; o < outer; o++) {
        int inner = 10;
        while (inner-- > 0) {
            do_work(array, (o * 10 + inner) % 100, o + inner);
        }
    }
    
    /* Loop 5: Another variant with explicit comparison */
    int m = iterations;
    while (m > 0) {
        do_work(array, m % 100, m);
        m = m - 1;
    }
}

/* Alternative test with different optimization pragma */
#pragma GCC push_options
#pragma GCC optimize ("O1")
NOOPT int test_loops_alt(int n, int *arr) {
    int sum = 0;
    
    /* Loop with explicit decrement and compare */
    for (int i = n; i != 0; i--) {
        arr[i % 64] = i;
        sum += arr[(i + 1) % 64];
    }
    
    /* Another loop with while */
    int j = n;
    while (j) {
        arr[j % 64] ^= j;
        j--;
    }
    
    return sum;
}
#pragma GCC pop_options

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 100) {
        fprintf(stderr, "Iterations should be >= 100\n");
        return 1;
    }
    
    /* Non-constant array to prevent optimization */
    int *array = (int*)malloc(100 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    /* Call the test functions */
    test_loops(iterations, array);
    
    int alt_result = test_loops_alt(iterations / 2, array);
    
    /* Use results to prevent dead code elimination */
    int final_sum = 0;
    for (int i = 0; i < 100; i++) {
        final_sum += array[i];
    }
    
    printf("Result: %d (alt: %d, side: %d)\n", 
           final_sum, alt_result, g_side_effect);
    
    free(array);
    return 0;
}
