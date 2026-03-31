/* loop_doloop_test.c
 * Test program to trigger specific RTL pattern in loop-doloop pass
 * Pattern: COMPARE (PLUS (reg, -1), const0_rtx)
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent loop elimination */
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
void test_signed_dec_ne_zero(unsigned int n) {
    /* Pattern: while (n-- != 0) */
    unsigned int i = n;
    while (i-- != 0) {
        volatile_sink = (int)i;
        global_sum += 1;
    }
}

__attribute__((noinline))
void test_unsigned_dec(unsigned int n) {
    /* Pattern: for (unsigned i = n; i != 0; i--) */
    for (unsigned int i = n; i != 0; i--) {
        volatile_sink = (int)i;
        global_sum += i & 0xFF;
    }
}

__attribute__((noinline))
void test_nested_loops(int n) {
    /* Outer loop with inner loop matching pattern */
    for (int outer = 0; outer < 3; outer++) {
        /* Inner loop with decrementing counter */
        int inner = n;
        while (inner > 0) {
            volatile_sink = inner + outer;
            global_sum++;
            inner--;
        }
    }
}

__attribute__((noinline))
void test_parameter_counter(int n) {
    /* Counter from parameter, prevents constant propagation */
    int counter = n;
    do {
        volatile_sink = counter;
        global_sum += counter % 7;
        counter--;
    } while (counter > 0);
}

__attribute__((noinline))
void test_mixed_types(int n) {
    /* Mix signed/unsigned with different comparison styles */
    
    /* Test 1: signed decrement with > 0 */
    int i = n;
    while (i > 0) {
        volatile_sink = i * 2;
        i--;
    }
    
    /* Test 2: unsigned decrement with != 0 */
    unsigned int j = (unsigned int)n;
    while (j != 0) {
        volatile_sink = (int)j;
        j--;
    }
}

__attribute__((noinline))
void test_array_access(int n) {
    /* Loop with array access to prevent optimization */
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Decrementing loop accessing array */
    for (int i = n; i > 0; i--) {
        volatile_sink = arr[i % 100];
        global_sum += arr[i % 100];
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
    test_signed_dec_ne_zero((unsigned int)base);
    test_unsigned_dec((unsigned int)(base / 2));
    test_nested_loops(base % 50 + 10);
    test_parameter_counter(base % 30 + 5);
    test_mixed_types(base % 40 + 8);
    test_array_access(base % 60 + 20);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", global_sum);
    
    /* Return non-zero if any loops executed */
    return global_sum == 0 ? 1 : 0;
}
