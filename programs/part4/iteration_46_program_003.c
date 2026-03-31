/* loop_doloop_test.c
 * Test program targeting loop-doloop.cc uncovered lines 136-150
 * Compile with: gcc -O2 -fno-unroll-loops -fno-peel-loops loop_doloop_test.c -o loop_test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
static volatile int volatile_sink;
static int global_sum = 0;

/* Non-inline functions to ensure RTL generation */
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
void test_signed_dec_neq_zero(int n) {
    /* Pattern: while (n != 0) with explicit decrement */
    while (n != 0) {
        volatile_sink = n;
        global_sum += n;
        n--;  /* Decrement after use */
    }
}

__attribute__((noinline))
void test_unsigned_dec(unsigned int n) {
    /* Pattern: unsigned counter with != 0 comparison */
    unsigned int i = n;
    while (i != 0) {
        volatile_sink = (int)i;
        global_sum += (int)i;
        i--;  /* Decrement to zero */
    }
}

__attribute__((noinline))
void test_pre_decrement(int n) {
    /* Pattern: --i style decrement */
    int i = n;
    while (i > 0) {
        volatile_sink = i;
        global_sum += i;
        --i;  /* Pre-decrement */
    }
}

__attribute__((noinline))
void test_nested_loops(int n) {
    /* Outer loop */
    for (int outer = 3; outer > 0; outer--) {
        /* Inner loop matching the target pattern */
        int inner = n;
        while (inner > 0) {
            volatile_sink = inner + outer;
            global_sum += inner;
            inner--;  /* Decrement inner counter */
        }
    }
}

__attribute__((noinline))
void test_do_while(int n) {
    /* do-while version that should still generate compare-after-decrement */
    int i = n;
    if (i <= 0) return;
    
    do {
        volatile_sink = i;
        global_sum += i;
        i--;
    } while (i > 0);
}

__attribute__((noinline))
void test_register_counter(int n) {
    /* Force counter to stay in register */
    register int counter asm("r12") = n;
    
    while (counter > 0) {
        volatile_sink = counter;
        global_sum += counter;
        counter--;
    }
}

__attribute__((noinline))
void test_mixed_operations(int n) {
    /* More complex loop body, but same decrement pattern */
    int i = n;
    int temp = 0;
    
    while (i > 0) {
        /* Multiple operations to prevent simplification */
        volatile_sink = i;
        temp = i * 2;
        global_sum += temp;
        i--;  /* Critical: decrement by 1 */
    }
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to prevent constant propagation */
    int base_count = 100;
    if (argc > 1) {
        base_count = atoi(argv[1]);
        if (base_count <= 0) base_count = 100;
    }
    
    /* Vary loop counts to hit different paths */
    test_signed_dec_gt_zero(base_count);
    test_signed_dec_neq_zero(base_count / 2);
    test_unsigned_dec((unsigned int)(base_count / 3));
    test_pre_decrement(base_count / 4);
    test_nested_loops(base_count / 5);
    test_do_while(base_count / 6);
    test_register_counter(base_count / 7);
    test_mixed_operations(base_count / 8);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (volatile sink: %d)\n", global_sum, volatile_sink);
    
    return (global_sum > 0) ? 0 : 1;
}
