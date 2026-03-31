/* loop_doloop_test.c
 * Test program to trigger specific RTL pattern in loop-doloop pass
 * Pattern: COMPARE (PLUS (reg, -1), 0)
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent loop removal */
static volatile int global_counter = 0;
static volatile int global_result = 0;

/* Non-inline functions to ensure RTL generation */
__attribute__((noinline)) 
void test_signed_dec_gt_zero(int n) {
    /* Pattern: for (int i = n; i > 0; i--) */
    for (int i = n; i > 0; i--) {
        /* Non-removable operation */
        global_counter += i;
    }
}

__attribute__((noinline))
void test_signed_dec_ne_zero(int n) {
    /* Pattern: while (n-- != 0) */
    int i = n;
    while (i != 0) {
        global_result ^= i;
        i--;
    }
}

__attribute__((noinline))
void test_unsigned_dec(unsigned int n) {
    /* Pattern: unsigned counter decrementing to zero */
    unsigned int i = n;
    while (i > 0) {
        global_counter += (int)i;
        i--;
    }
}

__attribute__((noinline))
void test_post_decrement(unsigned int n) {
    /* Pattern: while (n-- > 0) */
    unsigned int i = n;
    while (i-- > 0) {
        global_result |= (1 << (i & 0xF));
    }
}

__attribute__((noinline))
void test_nested_loops(int outer, int inner) {
    /* Outer loop with inner loop matching pattern */
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrementing counter */
        int i = inner;
        while (i > 0) {
            global_counter += o * i;
            i--;
        }
    }
}

__attribute__((noinline))
void test_do_while(int n) {
    /* do-while version */
    int i = n;
    if (i <= 0) return;
    
    do {
        global_result += i;
        i--;
    } while (i > 0);
}

__attribute__((noinline))
void test_parameter_counter(int start) {
    /* Counter from parameter, not constant */
    for (int i = start; i > 0; i--) {
        /* Mix of operations to prevent optimization */
        asm volatile("" : "+r" (global_counter) : "r" (i));
        global_counter = global_counter ^ (i * 7);
    }
}

__attribute__((noinline))
void test_array_access(int n, int* arr) {
    /* Loop with array access */
    for (int i = n; i > 0; i--) {
        arr[i-1] = i * 2;
        global_counter += arr[i-1];
    }
}

__attribute__((noinline))
void test_mixed_types(unsigned short n) {
    /* Different integer type */
    unsigned short i = n;
    while (i != 0) {
        global_result += i;
        i--;
    }
}

int main(int argc, char* argv[]) {
    /* Use command line arguments to prevent constant propagation */
    int base = 100;
    if (argc > 1) {
        base = atoi(argv[1]);
        if (base <= 0) base = 100;
    }
    
    /* Vary loop sizes to test different cases */
    test_signed_dec_gt_zero(base);
    test_signed_dec_ne_zero(base + 1);
    test_unsigned_dec((unsigned int)base + 2);
    test_post_decrement((unsigned int)base + 3);
    test_nested_loops(5, base / 5);
    test_do_while(base + 4);
    test_parameter_counter(base + 5);
    
    /* Test with array */
    int* arr = (int*)malloc(sizeof(int) * (base + 10));
    test_array_access(base + 6, arr);
    free(arr);
    
    test_mixed_types((unsigned short)(base % 65535));
    
    /* Ensure loops executed by checking results */
    printf("Global counter: %d\n", global_counter);
    printf("Global result: %d\n", global_result);
    
    /* Return non-zero if loops executed */
    return (global_counter != 0 || global_result != 0) ? 0 : 1;
}
