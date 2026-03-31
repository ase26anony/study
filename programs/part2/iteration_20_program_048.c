/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical variables */
static volatile int g_volatile_sink = 0;
static int g_array[1024];

/* External function to prevent inlining */
__attribute__((noinline, noclone))
static void use_value(int value) {
    g_volatile_sink = value;
}

/* Function containing loops with decrementing counters */
__attribute__((noinline, optimize("O1")))
static int test_loops(int iterations, int* results) {
    int i, j;
    int sum = 0;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        /* Non-trivial operation that can't be optimized away */
        results[i % 1024] = i * 2;
        use_value(i);
    }
    
    /* Loop variant 2: do-while with pre-decrement */
    j = iterations;
    if (j > 0) {
        do {
            results[j % 1024] += j;
            use_value(j);
        } while (--j > 0);
    }
    
    /* Loop variant 3: while with post-decrement */
    int k = iterations;
    while (k--) {
        results[k % 1024] *= 2;
        use_value(k);
        /* Nested loop to increase complexity */
        int m = 10;
        while (m-- > 0) {
            sum += results[(k + m) % 1024];
        }
    }
    
    /* Loop variant 4: Another for loop with different structure */
    for (int n = iterations; n != 0; n = n - 1) {
        results[n % 1024] = results[n % 1024] ^ 0x55;
        use_value(n);
        /* Additional operation to prevent simplification */
        for (int p = 5; p > 0; p--) {
            sum += p;
        }
    }
    
    return sum;
}

/* Another function with different optimization attributes */
#pragma GCC push_options
#pragma GCC optimize("O1")
__attribute__((noinline))
static int test_more_loops(int limit, int* arr) {
    int total = 0;
    
    /* Loop with compound condition */
    for (int cnt = limit; cnt > 0; cnt--) {
        arr[cnt & 1023] = cnt;
        total += arr[cnt & 1023];
        
        /* Force register usage */
        asm volatile("" : "+r"(total) : : "memory");
    }
    
    /* Simple decrement loop */
    int x = limit;
    while (x > 0) {
        arr[x & 1023] += x;
        total ^= arr[x & 1023];
        x--;
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
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 1024; i++) {
        g_array[i] = i + 1;
    }
    
    /* Call the test functions */
    int result1 = test_loops(iterations, g_array);
    int result2 = test_more_loops(iterations / 2, g_array);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Volatile sink: %d\n", g_volatile_sink);
    
    /* Additional check to use array values */
    int final_check = 0;
    for (int i = 0; i < 100; i++) {
        final_check ^= g_array[i];
    }
    printf("Final check: %d\n", final_check);
    
    return 0;
}
