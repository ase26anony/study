/* loop_doloop_test.c
 * Test program targeting uncovered lines 136-150 in loop-doloop.cc.gcov
 * Compile with: gcc -O2 -fno-unroll-loops -fno-peel-loops loop_doloop_test.c -o loop_test
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
void test_signed_dec_ne_zero(int n) {
    /* Pattern: while (n != 0) { n--; ... } */
    while (n != 0) {
        global_counter += n;
        global_array[n & 1023] = n;
        n--;
    }
}

__attribute__((noinline))
void test_unsigned_dec_ne_zero(unsigned int n) {
    /* Pattern: while (n-- != 0) */
    while (n-- != 0) {
        global_counter += 1;
        global_array[global_counter & 1023] = global_counter;
    }
}

__attribute__((noinline))
void test_pre_decrement(int n) {
    /* Pattern: while (--n >= 0) - but adjusted for > 0 comparison */
    int i = n;
    while (i > 0) {
        global_counter += i;
        global_array[i & 1023] = i;
        --i;
    }
}

__attribute__((noinline))
void test_nested_loops(int n, int m) {
    /* Outer loop with inner loop matching the pattern */
    for (int i = 0; i < n; i++) {
        /* Inner loop: decrement and compare to zero */
        int j = m;
        while (j > 0) {
            global_counter += i * j;
            global_array[(i * j) & 1023] = i + j;
            j--;
        }
    }
}

__attribute__((noinline))
void test_do_while_decrement(int n) {
    /* do-while version */
    int i = n;
    if (i > 0) {
        do {
            global_counter += i;
            global_array[i & 1023] = i;
            i--;
        } while (i > 0);
    }
}

__attribute__((noinline))
void test_parameter_counter(int n) {
    /* Counter comes from parameter, not constant */
    for (int i = n; i > 0; i--) {
        /* Use both volatile and non-volatile operations */
        int temp = global_counter;
        temp += i * 2;
        global_counter = temp;
        global_array[i & 1023] = temp;
    }
}

__attribute__((noinline))
void test_mixed_operations(int n) {
    /* More complex loop body to prevent optimization */
    int i = n;
    while (i > 0) {
        /* Multiple operations to create interesting RTL */
        global_counter++;
        global_array[global_counter & 1023] = i;
        
        /* Conditional to prevent simple analysis */
        if (global_counter & 1) {
            global_array[(i * 3) & 1023] = global_counter;
        }
        
        i--;
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to prevent constant propagation */
    int base = 100;
    if (argc > 1) {
        base = atoi(argv[1]);
        if (base <= 0) base = 100;
    }
    
    /* Vary loop counts to test different patterns */
    test_signed_dec_gt_zero(base);
    test_signed_dec_ne_zero(base + 10);
    test_unsigned_dec_ne_zero((unsigned int)(base + 20));
    test_pre_decrement(base + 30);
    test_nested_loops(base / 10, base / 5);
    test_do_while_decrement(base + 40);
    test_parameter_counter(base + 50);
    test_mixed_operations(base + 60);
    
    /* Return value based on results to ensure loops aren't dead code */
    printf("Final counter: %d\n", global_counter);
    return (global_counter > 0) ? 0 : 1;
}
