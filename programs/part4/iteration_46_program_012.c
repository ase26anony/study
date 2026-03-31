/* test_loop_doloop.c
 * Program designed to trigger specific RTL pattern in GCC's loop-doloop pass
 * Pattern: COMPARE (PLUS (reg, -1), const0_rtx)
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent loop elimination */
static volatile int volatile_sink = 0;
static int global_sum = 0;

/* Non-inline functions to ensure RTL generation */
__attribute__((noinline))
void test_signed_dec_gt(int n) {
    /* Pattern: for (int i = n; i > 0; i--) */
    for (int i = n; i > 0; i--) {
        /* Non-removable operation */
        volatile_sink = i;
        global_sum += i;
    }
}

__attribute__((noinline))
void test_signed_dec_ne(int n) {
    /* Pattern: while (n-- != 0) */
    int i = n;
    while (i != 0) {
        volatile_sink = i;
        global_sum += i * 2;
        i--;
    }
}

__attribute__((noinline))
void test_unsigned_dec(unsigned int n) {
    /* Pattern: unsigned counter with != 0 comparison */
    unsigned int i = n;
    while (i != 0) {
        volatile_sink = (int)i;
        global_sum += (int)i;
        i--;
    }
}

__attribute__((noinline))
void test_nested_loops(int n) {
    /* Outer loop with inner loop matching pattern */
    for (int outer = 0; outer < 3; outer++) {
        /* Inner loop with decrementing counter */
        int inner = n;
        while (inner > 0) {
            volatile_sink = outer * 100 + inner;
            global_sum += inner;
            inner--;
        }
    }
}

__attribute__((noinline))
void test_do_while(int n) {
    /* do-while that should compile to similar pattern */
    int i = n;
    if (i > 0) {
        do {
            volatile_sink = i;
            global_sum += i * 3;
            i--;
        } while (i > 0);
    }
}

__attribute__((noinline))
void test_param_counter(int start, int end) {
    /* Counter from parameters, not constants */
    for (int i = start; i > end; i--) {
        volatile_sink = i;
        global_sum += i * 4;
    }
}

__attribute__((noinline))
void test_mixed_types(short n) {
    /* Different integer type */
    short i = n;
    while (i > 0) {
        volatile_sink = i;
        global_sum += i;
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
    
    /* Call test functions with varying arguments */
    test_signed_dec_gt(base);
    test_signed_dec_ne(base / 2);
    test_unsigned_dec((unsigned int)(base * 2));
    test_nested_loops(base / 3);
    test_do_while(base / 4);
    test_param_counter(base, base / 5);
    test_mixed_types((short)(base / 6));
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (volatile sink: %d)\n", global_sum, volatile_sink);
    
    return (global_sum > 0) ? 0 : 1;
}
