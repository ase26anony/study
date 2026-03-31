/* loop-doloop-test.c
 * Test program to trigger specific RTL pattern in loop-doloop pass
 * Pattern: COMPARE with (PLUS reg -1) against const0_rtx
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
        volatile_sink = i;  /* Non-removable operation */
        global_sum += i;
    }
}

__attribute__((noinline))
void test_signed_dec_ne_zero(int n) {
    /* Pattern: while (n-- != 0) */
    while (n-- != 0) {
        volatile_sink = n;
        global_sum += n;
    }
}

__attribute__((noinline))
void test_unsigned_dec(unsigned int n) {
    /* Pattern: unsigned counter with != 0 comparison */
    unsigned int i = n;
    while (i != 0) {
        volatile_sink = i;
        global_sum += i;
        i--;
    }
}

__attribute__((noinline))
void test_nested_loops(int outer, int inner) {
    /* Outer loop with inner loop matching pattern */
    for (int o = 0; o < outer; o++) {
        /* Inner loop: decrement and compare to zero */
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
    /* do-while version that may generate similar pattern */
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
void test_param_counter(int start) {
    /* Counter from parameter, not constant */
    for (int i = start; i > 0; i--) {
        /* Array access to prevent optimization */
        static int arr[100];
        arr[i % 100] = i;
        volatile_sink = arr[i % 100];
    }
}

__attribute__((noinline))
void test_mixed_types(short n) {
    /* Different integer type */
    short i = n;
    while (i > 0) {
        volatile_sink = i;
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
    
    /* Vary loop counts to hit different paths */
    test_signed_dec_gt_zero(base);
    test_signed_dec_ne_zero(base / 2);
    test_unsigned_dec((unsigned int)(base * 2));
    test_nested_loops(base / 10, base / 5);
    test_do_while(base / 3);
    test_param_counter(base + 7);
    test_mixed_types((short)(base % 100));
    
    /* Return value based on results to ensure loops aren't dead code */
    printf("Result: %d\n", global_sum);
    return (global_sum > 0) ? 0 : 1;
}
