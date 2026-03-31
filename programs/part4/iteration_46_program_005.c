/* loop_doloop_test.c
 * Test program targeting uncovered lines 136-150 in loop-doloop.cc.gcov
 * Compile with: gcc -O2 -fdump-rtl-loop-doloop -fno-unroll-loops -march=x86-64 loop_doloop_test.c -o loop_test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent loop elimination */
static volatile int global_counter = 0;
static int global_array[1024];

/* Non-inline functions to ensure RTL generation */
__attribute__((noinline)) 
void test_signed_dec_gt_zero(int n) {
    /* Pattern: for (int i = n; i > 0; i--) */
    for (int i = n; i > 0; i--) {
        /* Non-removable operation */
        global_counter += i;
        global_array[i & 1023] = i;
    }
}

__attribute__((noinline))
void test_signed_dec_ne_zero(unsigned int n) {
    /* Pattern: while (n-- != 0) */
    while (n-- != 0) {
        /* Force register usage */
        global_counter += (int)n;
        global_array[global_counter & 1023] = n;
    }
}

__attribute__((noinline))
void test_unsigned_dec(unsigned int n) {
    /* Pattern: for (unsigned i = n; i != 0; i--) */
    for (unsigned int i = n; i != 0; i--) {
        global_counter += 1;
        global_array[i & 1023] = (int)i;
    }
}

__attribute__((noinline))
void test_nested_loops(int n) {
    /* Outer loop with inner decrementing loop */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop matching the target pattern */
        int inner = n;
        while (inner > 0) {
            global_counter += outer + inner;
            global_array[(outer * inner) & 1023] = inner;
            inner--;
        }
    }
}

__attribute__((noinline))
void test_do_while(int n) {
    /* do-while version that should generate similar pattern */
    int i = n;
    if (i > 0) {
        do {
            global_counter += i;
            global_array[i & 1023] = i;
        } while (--i > 0);
    }
}

__attribute__((noinline))
void test_parameter_counter(int start, int end) {
    /* Counter from parameters, not constant */
    for (int i = start; i > end; i--) {
        global_counter += i * 2;
        global_array[i & 1023] = i * 3;
    }
}

__attribute__((noinline))
void test_mixed_types(short n) {
    /* Different type, but should still generate compare pattern */
    int i = n;
    while (i > 0) {
        global_counter += i;
        i--;
    }
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to prevent constant propagation */
    int base = 100;
    if (argc > 1) {
        base = atoi(argv[1]);
        if (base <= 0) base = 100;
    }
    
    /* Call all test functions with varying arguments */
    test_signed_dec_gt_zero(base);
    test_signed_dec_ne_zero(base);
    test_unsigned_dec(base);
    test_nested_loops(base / 10);
    test_do_while(base);
    test_parameter_counter(base, base / 2);
    test_mixed_types(base);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (array[0]=%d)\n", 
           global_counter, 
           global_array[0]);
    
    return global_counter != 0 ? 0 : 1;
}
