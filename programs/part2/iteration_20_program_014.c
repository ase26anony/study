/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop_doloop
 * Or for RISC-V: riscv64-linux-gnu-gcc -O1 -march=rv64gc -fdump-rtl-expand test_loop_doloop.c -o test_loop_doloop
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of loops */
#define NOOPT __attribute__((optimize("O1")))

/* External function to prevent inlining */
extern void use_value(int val) __attribute__((noinline, noclone));

/* Implementation of external function */
void use_value(int val) {
    /* Use volatile to prevent optimization */
    static volatile int sink;
    sink = val;
}

/* Function containing the loops we want to test */
NOOPT void test_loops(int iterations, int* array) {
    int i, j, k, m;
    
    /* Loop 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        /* Non-trivial operation that can't be optimized away */
        array[i % 256] = i;
        use_value(array[i % 256]);
    }
    
    /* Loop 2: do-while loop with decrement */
    j = iterations;
    if (j > 0) {
        do {
            array[j % 256] = j * 2;
            use_value(array[j % 256]);
        } while (--j > 0);
    }
    
    /* Loop 3: while loop with post-decrement */
    k = iterations;
    while (k--) {
        array[k % 256] = k * 3;
        use_value(array[k % 256]);
    }
    
    /* Loop 4: Nested loops - inner loop uses decrementing counter */
    for (m = 10; m > 0; m--) {
        int inner = iterations % 100;
        while (inner-- > 0) {
            array[(m + inner) % 256] = m + inner;
            use_value(array[(m + inner) % 256]);
        }
    }
    
    /* Loop 5: Another variant with explicit decrement */
    int n = iterations;
    while (n > 0) {
        array[n % 256] = n * 4;
        use_value(array[n % 256]);
        n = n - 1;  /* Explicit decrement instead of n-- */
    }
}

/* Alternative version with pragma for specific optimization level */
#pragma GCC push_options
#pragma GCC optimize ("O1")
void test_loops_pragma(int iterations, int* array) {
    int i = iterations;
    
    /* Simple decrementing loop that should generate the target pattern */
    while (i > 0) {
        /* Volatile operations to prevent optimization */
        volatile int* ptr = &array[i & 255];
        *ptr = i;
        i--;
    }
    
    /* Another variant with different syntax */
    for (int j = iterations; j > 0; j -= 1) {
        array[j & 255] = j * j;
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
    
    /* Use heap allocation to prevent stack optimization */
    int* array = (int*)malloc(256 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 256; i++) {
        array[i] = i + 1;
    }
    
    /* Test both versions */
    test_loops(iterations, array);
    test_loops_pragma(iterations, array);
    
    /* Compute and print a result to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += array[i];
    }
    printf("Result: %d\n", sum);
    
    free(array);
    return 0;
}
