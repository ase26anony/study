/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop_doloop
 * Or with: gcc -O2 -fsanitize=address -fno-tree-vectorize -fno-unroll-loops test_loop_doloop.c -o test_loop_doloop
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of critical loops */
#pragma GCC push_options
#pragma GCC optimize("O1")

/* Volatile variable to prevent dead code elimination */
static volatile int volatile_sink;

/* External function to create side effects */
void __attribute__((noinline, noclone)) side_effect(int x) {
    volatile_sink = x;
}

/* Function containing the loops we want to test */
__attribute__((noinline, optimize("O1")))
void test_loops(int iterations, int* array) {
    int i, j, k, m;
    int sum = 0;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        /* Non-trivial operation with side effect */
        array[i % 256] = i;
        side_effect(array[i % 256]);
        sum += i;
    }
    
    /* Loop variant 2: do-while loop with decrement */
    j = iterations;
    if (j > 0) {
        do {
            array[j % 256] = j * 2;
            side_effect(array[j % 256]);
            sum += j;
        } while (--j > 0);
    }
    
    /* Loop variant 3: while loop with post-decrement */
    k = iterations;
    while (k--) {
        array[k % 256] = k * 3;
        side_effect(array[k % 256]);
        sum += k;
    }
    
    /* Loop variant 4: Nested loops with inner decrementing loop */
    m = iterations / 4;
    for (i = 0; i < 4; i++) {
        int inner = m;
        while (inner-- > 0) {
            array[(i * m + inner) % 256] = i + inner;
            side_effect(array[(i * m + inner) % 256]);
            sum += i + inner;
        }
    }
    
    /* Store final result to prevent elimination */
    array[0] = sum;
    side_effect(sum);
}

#pragma GCC pop_options

/* Another test function with different optimization level */
__attribute__((noinline, optimize("O2")))
void test_loops_opt2(int iterations, int* array) {
    int i = iterations;
    
    /* Simple decrementing loop that might generate the pattern at O2 */
    while (i > 0) {
        array[i % 128] = i;
        /* Use inline asm to prevent optimization */
        __asm__ volatile ("" : : "r"(array[i % 128]) : "memory");
        i--;
    }
}

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
    
    /* Use heap array to prevent stack-based optimizations */
    int* array1 = (int*)malloc(256 * sizeof(int));
    int* array2 = (int*)malloc(128 * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-zero values */
    memset(array1, 0xAA, 256 * sizeof(int));
    memset(array2, 0xBB, 128 * sizeof(int));
    
    /* Test with O1 optimization (most likely to trigger the pattern) */
    test_loops(iterations, array1);
    
    /* Also test with O2 but disabled vectorization/unrolling */
    test_loops_opt2(iterations / 2, array2);
    
    /* Compute checksum to ensure loops executed */
    int checksum1 = 0, checksum2 = 0;
    for (int i = 0; i < 256; i++) checksum1 ^= array1[i];
    for (int i = 0; i < 128; i++) checksum2 ^= array2[i];
    
    printf("Checksum 1: %d\n", checksum1);
    printf("Checksum 2: %d\n", checksum2);
    printf("Volatile sink: %d\n", volatile_sink);
    
    free(array1);
    free(array2);
    
    return 0;
}
