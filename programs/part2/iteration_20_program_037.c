/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
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
NOOPT void __attribute__((noinline, noclone)) 
side_effect(int value) {
    g_volatile_sink += value;
}

/* Array with non-constant access pattern */
NOOPT void process_array(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = i * 2;
    }
}

/* Main test function with multiple loop variants */
NOOPT void __attribute__((noinline)) 
test_loops(int iterations, int *arr) {
    int i, j;
    int local_counter = iterations;
    
    /* Variant 1: Basic for loop with decrement */
    /* Should generate: (reg + -1) COMPARE 0 */
    for (i = iterations; i > 0; i--) {
        side_effect(i);
        arr[i % 64] += i;  /* Non-constant array index */
    }
    
    /* Variant 2: Do-while with pre-decrement */
    /* Another chance for the pattern */
    j = iterations;
    do {
        side_effect(j);
        arr[j % 64] -= 1;
        g_volatile_counter++;
    } while (--j > 0);
    
    /* Variant 3: While loop with post-decrement */
    /* Different syntax, same pattern potential */
    local_counter = iterations;
    while (local_counter--) {
        side_effect(local_counter);
        arr[local_counter % 64] *= 2;
    }
    
    /* Variant 4: Nested loops - inner loop decrements */
    /* Increases chances of pattern matching */
    for (i = 0; i < 5; i++) {
        int inner = iterations / 10;
        while (inner-- > 0) {
            side_effect(inner + i);
            arr[(inner + i) % 64] = inner;
        }
    }
    
    /* Variant 5: Complex decrement pattern */
    /* Mix of operations that might survive to RTL */
    int k = iterations;
    int sum = 0;
    for (; k > 0; k--) {
        /* Conditional to prevent simple transformations */
        if (k & 1) {
            sum += arr[k % 64];
        } else {
            sum -= arr[k % 64];
        }
        side_effect(sum);
    }
    g_volatile_sink += sum;
}

/* Alternative: Function with attribute to control optimization */
#pragma GCC push_options
#pragma GCC optimize("O1")
NOOPT void test_loops_alt(int n, int *arr) {
    /* Another set of loops in differently optimized function */
    int m = n;
    
    /* Simple countdown loop */
    while (m > 0) {
        arr[m % 32] = m * 3;
        side_effect(arr[m % 32]);
        m--;
    }
    
    /* Do-while countdown */
    m = n / 2;
    do {
        arr[m % 32] += m;
        g_volatile_counter += arr[m % 32];
    } while (--m > 0);
}
#pragma GCC pop_options

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 20) {
        fprintf(stderr, "Need at least 20 iterations\n");
        return 1;
    }
    
    /* Non-constant sized array */
    int *array = malloc(64 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant pattern */
    process_array(array, 64);
    
    /* Reset volatile counter */
    g_volatile_sink = 0;
    g_volatile_counter = 0;
    
    /* Test the main loop function */
    test_loops(iterations, array);
    
    /* Test alternative function */
    test_loops_alt(iterations / 2, array);
    
    /* Use results to prevent elimination */
    int total = 0;
    for (int i = 0; i < 64; i++) {
        total += array[i];
    }
    
    printf("Result: array_sum=%d, volatile_sink=%d, volatile_counter=%d\n",
           total, g_volatile_sink, g_volatile_counter);
    
    free(array);
    return 0;
}
