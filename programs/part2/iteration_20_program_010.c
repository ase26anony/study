/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical loops */
#define NOOPT __attribute__((optimize("O1"), noinline, noclone))

/* Volatile variables to prevent dead code elimination */
static volatile int g_volatile_sink = 0;
static volatile int g_volatile_counter = 0;

/* External function to create side effects */
NOOPT void external_side_effect(int value) {
    g_volatile_sink += value;
}

/* Array with non-constant access pattern */
NOOPT void modify_array(int* arr, int size, int value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i;
    }
}

/* Test function containing multiple loop variants */
NOOPT int test_loops(int iterations, int* results) {
    int sum = 0;
    int local_iter = iterations;
    
    /* VARIANT 1: Basic for loop with decrementing counter */
    /* Should generate: (reg + -1) COMPARE 0 */
    for (int i = iterations; i > 0; i--) {
        external_side_effect(i);
        results[iterations - i] = i * 2;
        sum += i;
    }
    
    /* VARIANT 2: do-while loop with pre-decrement */
    int n = local_iter;
    if (n > 0) {
        do {
            external_side_effect(n);
            results[local_iter - n] += n;
            sum += n;
        } while (--n > 0);
    }
    
    /* VARIANT 3: while loop with post-decrement */
    int m = local_iter;
    while (m--) {
        external_side_effect(m);
        results[local_iter - m - 1] *= (m + 1);
        sum += m;
        
        /* Nested inner loop with decrementing counter */
        int inner = 5;
        while (inner-- > 0) {
            g_volatile_counter++;
        }
    }
    
    /* VARIANT 4: Complex decrement pattern */
    int k = local_iter;
    int count = 0;
    for (; k > 0; k -= 1) {
        if (count++ % 3 == 0) {
            external_side_effect(k);
        }
        results[local_iter - k] -= k;
        sum -= k;
    }
    
    return sum;
}

/* Another test function with different optimization context */
#pragma GCC push_options
#pragma GCC optimize("O1")
NOOPT int test_more_loops(int base, int* buffer) {
    int total = 0;
    int limit = base;
    
    /* Loop with volatile condition to prevent optimization */
    volatile int vol_limit = limit;
    for (int j = vol_limit; j > 0; j--) {
        buffer[j] = j * j;
        total += buffer[j];
        
        /* Additional operation to create more RTL */
        asm volatile("" : "+r"(j) : : "memory");
    }
    
    /* Loop with function call in condition */
    int p = limit;
    while (1) {
        external_side_effect(p);
        buffer[p] = total;
        total >>= 1;
        if (--p <= 0) break;
    }
    
    return total;
}
#pragma GCC pop_options

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iteration_count>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 10) {
        fprintf(stderr, "Iteration count must be > 10\n");
        return 1;
    }
    
    /* Use dynamic allocation to prevent constant propagation */
    int* results = (int*)malloc(iterations * sizeof(int));
    if (!results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    modify_array(results, iterations, 42);
    
    /* Execute test functions */
    int sum1 = test_loops(iterations, results);
    int sum2 = test_more_loops(iterations / 2, results);
    
    /* Use results to prevent dead code elimination */
    int final_result = 0;
    for (int i = 0; i < iterations && i < 10; i++) {
        final_result ^= results[i];
    }
    
    final_result += sum1 + sum2 + g_volatile_sink + g_volatile_counter;
    
    printf("Result: %d (checksum: %08x)\n", 
           final_result, 
           (unsigned int)final_result);
    
    free(results);
    return 0;
}
