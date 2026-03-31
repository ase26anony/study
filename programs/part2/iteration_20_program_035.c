/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical loops */
#define NOOPT __attribute__((optimize("O1")))

/* Volatile variables to prevent dead code elimination */
static volatile int g_volatile_sink = 0;
static volatile int g_volatile_counter = 0;

/* External function to create side effects */
NOOPT static void side_effect(int value) {
    g_volatile_sink += value;
}

/* Function containing the loops we want to test */
NOOPT static int test_loops(int iterations, int* array) {
    int i, j;
    int result = 0;
    
    /* Loop 1: Basic for loop with decrementing counter */
    for (i = iterations; i > 0; i--) {
        array[i % 256] = i;  /* Non-constant array index */
        side_effect(i);
        result += array[(i + 1) % 256];
    }
    
    /* Loop 2: Do-while loop with decrement */
    j = iterations;
    do {
        array[j % 256] = j * 2;
        side_effect(j);
        result -= array[(j + 2) % 256];
        j--;
    } while (j > 0);
    
    /* Loop 3: While loop with post-decrement */
    int k = iterations;
    while (k--) {
        array[k % 256] = k * 3;
        side_effect(k);
        result ^= array[(k + 3) % 256];
    }
    
    /* Loop 4: Nested loops - inner loop uses decrementing counter */
    int m, n;
    for (m = 0; m < 10; m++) {
        n = iterations / 10;
        while (n-- > 0) {
            array[(m + n) % 256] = m * n;
            side_effect(m + n);
            result += array[(m * n) % 256];
        }
    }
    
    return result;
}

/* Alternative version with different optimization pragma */
#pragma GCC push_options
#pragma GCC optimize ("O1")
NOOPT static int test_loops_alt(int iterations, int* array) {
    int sum = 0;
    int count = iterations;
    
    /* Another variant: while loop with pre-decrement */
    while (--count > 0) {
        array[count % 128] = count * 5;
        g_volatile_counter += count;
        sum += array[(count * 2) % 128];
    }
    
    return sum;
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
        array[i] = i * 3 + iterations;
    }
    
    /* Call the test functions */
    int result1 = test_loops(iterations, array);
    int result2 = test_loops_alt(iterations, array);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Volatile sink: %d\n", g_volatile_sink);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    free(array);
    return 0;
}
