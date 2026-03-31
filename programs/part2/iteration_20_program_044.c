/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop_doloop
 * Or with RISC-V target: gcc -O1 -march=rv64gc -fdump-rtl-expand test_loop_doloop.c -o test_loop_doloop
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of critical loops */
#define NOOPT __attribute__((optimize("O1")))

/* External function to prevent inlining */
void __attribute__((noinline, noclone)) side_effect(int *p) {
    *p += 1;
}

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* Array with external linkage to prevent constant propagation */
int extern_array[1000];

/* Function containing the loops we want to test */
NOOPT void test_loops(int iterations, int *array) {
    int i, j;
    
    /* Loop Variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        /* Non-trivial operation that can't be optimized away */
        array[i % 1000] = i;
        global_counter++;
        side_effect(&array[i % 1000]);
    }
    
    /* Loop Variant 2: do-while with pre-decrement */
    j = iterations;
    if (j > 0) {
        do {
            array[j % 1000] = j * 2;
            global_counter += 2;
            side_effect(&array[j % 1000]);
        } while (--j > 0);
    }
    
    /* Loop Variant 3: while loop with post-decrement */
    int k = iterations;
    while (k--) {
        array[k % 1000] = k * 3;
        global_counter += 3;
        side_effect(&array[k % 1000]);
    }
    
    /* Loop Variant 4: Nested loops with inner decrementing counter */
    int outer = iterations / 10;
    for (int m = 0; m < outer; m++) {
        int inner = 10;
        while (inner-- > 0) {
            array[(m * 10 + inner) % 1000] = m * inner;
            global_counter += 4;
            side_effect(&array[(m * 10 + inner) % 1000]);
        }
    }
    
    /* Loop Variant 5: Another for loop with different comparison */
    for (int n = iterations; n != 0; n--) {
        array[n % 1000] = n * 5;
        global_counter += 5;
        side_effect(&array[n % 1000]);
    }
}

/* Alternative function with pragma for optimization control */
#pragma GCC push_options
#pragma GCC optimize ("O1")
void test_loops_pragma(int iterations, int *array) {
    int i = iterations;
    
    /* Simple decrementing loop that should generate clean RTL */
    while (i > 0) {
        /* Use volatile store to prevent optimization */
        *(volatile int*)&array[i % 1000] = i;
        i--;
    }
    
    /* Another variant with different syntax */
    for (int j = iterations; j > 0; --j) {
        array[j % 1000] = j * j;
    }
}
#pragma GCC pop_options

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 20) {
        printf("Iterations should be at least 20 for meaningful test\n");
        iterations = 100;  /* Default value */
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 1000; i++) {
        extern_array[i] = i * 3 + 7;
    }
    
    /* Reset global counter */
    global_counter = 0;
    
    /* Test the main loop function */
    test_loops(iterations, extern_array);
    
    /* Also test the pragma version */
    test_loops_pragma(iterations / 2, extern_array);
    
    /* Compute and print result to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += extern_array[i % 1000];
    }
    
    printf("Result: sum=%d, global_counter=%d\n", sum, global_counter);
    
    return 0;
}
