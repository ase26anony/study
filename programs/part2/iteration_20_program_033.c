/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of individual loops */
#define NOOPT __attribute__((optimize("O1")))

/* Volatile variables to prevent dead code elimination */
static volatile int volatile_sink = 0;
static volatile int volatile_counter = 0;

/* External function to prevent inlining */
void __attribute__((noinline, noclone)) 
external_side_effect(int x) {
    volatile_sink += x;
}

/* Function containing the loops we want to test */
NOOPT void test_loops(int iterations, int* array) {
    int i, j, k, m;
    
    /* Loop 1: Basic for loop with decrementing counter */
    for (i = iterations; i > 0; i--) {
        /* Non-trivial operation that can't be optimized away */
        array[i % 256] = i;
        external_side_effect(i);
    }
    
    /* Loop 2: Do-while loop with decrement */
    j = iterations;
    do {
        array[j % 256] = j * 2;
        external_side_effect(j);
    } while (--j > 0);
    
    /* Loop 3: While loop with post-decrement */
    k = iterations;
    while (k--) {
        array[k % 256] = k * 3;
        external_side_effect(k);
        /* Add some computation to prevent simplification */
        volatile_counter += (k & 1);
    }
    
    /* Loop 4: Nested loops with inner decrementing counter */
    for (m = 10; m > 0; m--) {
        int inner = iterations / 10;
        /* Inner loop with decrement */
        while (inner-- > 0) {
            array[(m * inner) % 256] = m + inner;
            external_side_effect(m + inner);
        }
    }
    
    /* Loop 5: Complex decrement with computation in condition */
    int n = iterations;
    int temp = 0;
    while (n-- > 0) {
        temp = n * n;
        array[n % 256] = temp;
        external_side_effect(temp);
    }
}

/* Alternative test with different optimization pragma */
#pragma GCC push_options
#pragma GCC optimize ("O1")
void test_loops_alt(int iterations, int* array) {
    int counter = iterations;
    
    /* Mix of different loop styles */
    while (counter) {
        array[counter % 128] = counter;
        external_side_effect(counter);
        counter--;
    }
    
    /* Another variant */
    for (int x = iterations; x != 0; x = x - 1) {
        array[x % 128] = x * x;
        external_side_effect(x);
    }
}
#pragma GCC pop_options

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iteration_count>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 100) {
        fprintf(stderr, "Iteration count should be >= 100 for meaningful test\n");
        iterations = 100;
    }
    
    /* Non-constant array to prevent optimization */
    int* array = (int*)malloc(256 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3 + 7;
    }
    
    /* Run the test loops */
    test_loops(iterations, array);
    test_loops_alt(iterations / 2, array);
    
    /* Compute a checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += array[i];
    }
    
    printf("Result: %d (volatile_sink=%d, volatile_counter=%d)\n", 
           sum, volatile_sink, volatile_counter);
    
    free(array);
    return 0;
}
