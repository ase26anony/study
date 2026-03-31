/* loop_doloop_test.c - Test program for loop-doloop pass coverage */

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
void test_signed_dec_neq_zero(int n) {
    /* Pattern: while (n != 0) with decrement in body */
    while (n != 0) {
        global_counter += n;
        global_array[n & 1023] = n;
        n--;  /* Decrement in body, not in condition */
    }
}

__attribute__((noinline))
void test_unsigned_dec(unsigned int n) {
    /* Pattern: while (n-- != 0) */
    while (n-- != 0) {
        global_counter += (int)n;
        global_array[n & 1023] = (int)n;
    }
}

__attribute__((noinline))
void test_pre_decrement(int n) {
    /* Pattern: while (--n > 0) */
    while (--n > 0) {
        global_counter += n;
        global_array[n & 1023] = n;
    }
}

__attribute__((noinline))
void test_nested_loops(int n) {
    /* Outer loop with inner decrementing loop */
    for (int outer = 0; outer < 5; outer++) {
        int inner = n;
        /* Inner loop matching the target pattern */
        while (inner > 0) {
            global_counter += inner + outer;
            global_array[(inner + outer) & 1023] = inner;
            inner--;
        }
    }
}

__attribute__((noinline))
void test_do_while(int n) {
    /* do-while with decrement and compare to zero */
    int i = n;
    if (i <= 0) return;
    
    do {
        global_counter += i;
        global_array[i & 1023] = i;
    } while (--i > 0);
}

__attribute__((noinline))
void test_register_counter(int n) {
    /* Force counter to stay in register */
    register int reg_counter = n;
    
    while (reg_counter > 0) {
        /* Complex enough to prevent optimization but simple enough
           to keep the decrement pattern */
        global_counter += reg_counter * 2;
        global_array[reg_counter & 1023] = reg_counter;
        reg_counter--;
    }
}

__attribute__((noinline))
void test_mixed_operations(int n) {
    /* Mix of operations that should still generate the target pattern */
    int counter = n;
    int sum = 0;
    
    for (; counter > 0; counter--) {
        /* Operation that uses counter but doesn't prevent the pattern */
        sum += counter;
        global_array[counter & 1023] = sum;
    }
    global_counter += sum;
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to prevent constant propagation */
    int base_count = 100;
    if (argc > 1) {
        base_count = atoi(argv[1]);
        if (base_count <= 0) base_count = 100;
    }
    
    /* Call all test functions with varying counts */
    test_signed_dec_gt_zero(base_count);
    test_signed_dec_neq_zero(base_count + 10);
    test_unsigned_dec((unsigned int)(base_count + 20));
    test_pre_decrement(base_count + 30);
    test_nested_loops(base_count / 2);
    test_do_while(base_count + 40);
    test_register_counter(base_count + 50);
    test_mixed_operations(base_count + 60);
    
    /* Verify something was computed */
    printf("Final counter value: %d\n", global_counter);
    printf("Sample array values: [0]=%d, [1]=%d, [100]=%d\n", 
           global_array[0], global_array[1], global_array[100]);
    
    return (global_counter > 0) ? 0 : 1;
}
