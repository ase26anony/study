/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of dummy function */
__attribute__((noinline)) 
static void dummy_side_effect(int value) {
    /* Use volatile to prevent elimination */
    volatile int sink = value;
    (void)sink;
}

/* External array to prevent dead code elimination */
extern volatile int external_buffer[1024];

/* Function A: Basic for loop with int counter */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_for_loop_decrement(int iterations) {
    volatile int sum = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sum += i;
        dummy_side_effect(i);
    }
    
    return sum;
}

/* Function B: While loop with unsigned counter */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_while_loop_decrement(unsigned int count) {
    volatile int result = 0;
    unsigned int n = count;
    
    /* while(n--) pattern */
    while (n--) {
        result ^= (int)n;  /* Simple computation */
        if (external_buffer) {
            /* Reference external to prevent optimization */
            dummy_side_effect(external_buffer[n % 1024]);
        }
    }
    
    return result;
}

/* Function C: Nested loops with inner decrement pattern */
__attribute__((optimize("O2", "no-unroll-loops", "no-peel-loops")))
int test_nested_loops(int outer, int inner) {
    volatile int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare */
        int i = inner;
        while (i--) {
            total += o * i;
            dummy_side_effect(total);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_param_counter(int count) {
    volatile int checksum = 0;
    int c = count;
    
    /* Force the decrement to happen in loop condition */
    do {
        checksum += c;
        dummy_side_effect(checksum);
    } while (c-- > 0);
    
    return checksum;
}

/* Function E: Multiple decrement patterns in same function */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_multiple_patterns(int a, int b) {
    volatile int acc = 0;
    
    /* First pattern */
    for (int i = a; i-- > 0;) {
        acc += i * 2;
    }
    
    /* Second pattern */
    unsigned int j = (unsigned int)b;
    while (j--) {
        acc -= (int)j;
        dummy_side_effect(acc);
    }
    
    return acc;
}

/* Main driver with verification */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test with non-constant values to prevent compile-time evaluation */
    int iter1 = 1000;
    unsigned int iter2 = 500;
    int outer = 10, inner = 50;
    int param = 200;
    int multi_a = 150, multi_b = 75;
    
    /* Volatile to prevent constant propagation of loop bounds */
    volatile int v1 = iter1;
    volatile unsigned int v2 = iter2;
    volatile int v3 = outer;
    volatile int v4 = inner;
    volatile int v5 = param;
    volatile int v6 = multi_a;
    volatile int v7 = multi_b;
    
    /* Execute all test functions */
    checksum += test_for_loop_decrement(v1);
    checksum += test_while_loop_decrement(v2);
    checksum += test_nested_loops(v3, v4);
    checksum += test_param_counter(v5);
    checksum += test_multiple_patterns(v6, v7);
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum is non-zero, loops executed successfully.\n");
    
    return 0;
}

/* Dummy external buffer definition to satisfy linker */
volatile int external_buffer[1024] = {0};
