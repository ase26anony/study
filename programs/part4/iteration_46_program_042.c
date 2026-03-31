/* loop-doloop-test.c
 * Test program to trigger loop-doloop pass pattern matching
 * Pattern: COMPARE (PLUS (reg, -1), const0_rtx)
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent loop elimination */
static volatile int volatile_sink = 0;
static int global_sum = 0;

/* Prevent inlining to ensure RTL generation */
__attribute__((noinline))
void test_signed_dec_gt_zero(int n) {
    /* Pattern: for (int i = n; i > 0; i--) */
    for (int i = n; i > 0; i--) {
        /* Non-removable operation */
        volatile_sink = i;
        global_sum += i;
    }
}

__attribute__((noinline))
void test_signed_dec_ne_zero(int n) {
    /* Pattern: while (n-- != 0) */
    while (n-- != 0) {
        /* Force register usage */
        int temp = n;
        volatile_sink = temp;
        global_sum += temp;
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
void test_nested_loops(int outer, int inner) {
    /* Outer loop to create context */
    for (int o = 0; o < outer; o++) {
        /* Inner loop matching the target pattern */
        int i = inner;
        while (i > 0) {
            volatile_sink = o * i;
            global_sum += o * i;
            i--;
        }
    }
}

__attribute__((noinline))
void test_do_while(int n) {
    /* do-while version that should generate similar pattern */
    int i = n;
    if (i > 0) {
        do {
            volatile_sink = i;
            global_sum += i;
            i--;
        } while (i > 0);
    }
}

__attribute__((noinline))
void test_parameter_counter(int start, int end) {
    /* Counter from parameters, not constant */
    for (int i = start; i > end; i--) {
        volatile_sink = i;
        global_sum += i;
    }
}

__attribute__((noinline))
void test_mixed_types(short n) {
    /* Different type to test various RTL representations */
    int i = n;
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
    
    /* Call test functions with varying parameters */
    test_signed_dec_gt_zero(base);
    test_signed_dec_ne_zero(base / 2);
    test_unsigned_dec((unsigned int)(base * 2));
    test_nested_loops(5, base / 5);
    test_do_while(base / 3);
    test_parameter_counter(base, 0);
    test_mixed_types((short)(base % 256));
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (volatile sink: %d)\n", global_sum, volatile_sink);
    
    return (global_sum > 0) ? 0 : 1;
}
