/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];
int extern_array[10000];

/* Volatile variable to prevent optimization */
volatile int volatile_sink = 0;

/* Non-inline function to create side effects */
__attribute__((noinline)) void dummy_side_effect(int value) {
    volatile_sink += value;
}

/* Function A: Basic for loop with int counter using decrement-and-compare pattern */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_basic_for_loop(int iterations) {
    int sum = 0;
    /* Canonical pattern: for (int i = N; i-- > 0;) */
    for (int i = iterations; i-- > 0;) {
        /* Simple side effect to prevent loop removal */
        sum += i;
        extern_array[i & 1023] = i;  /* Store to external array */
    }
    return sum;
}

/* Function B: While loop with unsigned int counter */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_while_loop(unsigned int count) {
    int result = 0;
    unsigned int n = count;
    
    /* Pattern: while (n--) */
    while (n--) {
        result += (int)n;
        dummy_side_effect(n);  /* Call noinline function */
    }
    return result;
}

/* Function C: Nested loops with inner loop using the pattern */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_nested_loops(int outer_iter, int inner_iter) {
    int total = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        int inner = inner_iter;
        /* Inner loop with decrement-and-compare pattern */
        while (inner--) {
            total += o * inner;
            volatile_sink += inner;  /* Use volatile variable */
        }
    }
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_param_loop(int count) {
    int checksum = 0;
    int c = count;
    
    /* Using the exact pattern needed for RTL matching */
    for (; c-- > 0;) {
        checksum ^= c;  /* Non-trivial computation */
        extern_array[c & 511] = checksum;
    }
    return checksum;
}

/* Function E: Loop with volatile bound to prevent early optimization */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_volatile_bound(void) {
    volatile int bound = 100;
    int accumulator = 0;
    int i = bound;
    
    /* Decrement-and-compare with volatile-influenced counter */
    while (i--) {
        accumulator += i * 3;
        dummy_side_effect(accumulator);
    }
    return accumulator;
}

/* Function F: Do-while style that should still match the pattern */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_do_while_pattern(int limit) {
    int counter = limit;
    int value = 0;
    
    if (counter <= 0) return 0;
    
    do {
        value += counter * 2;
        extern_array[counter & 255] = value;
    } while (counter-- > 1);  /* Note: counter-- > 1 creates the pattern */
    
    return value;
}

/* Main driver that executes all test functions */
int main(void) {
    int final_checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test A: Basic for loop */
    int result_a = test_basic_for_loop(1000);
    final_checksum ^= result_a;
    printf("Test A result: %d\n", result_a);
    
    /* Test B: While loop with unsigned */
    int result_b = test_while_loop(500);
    final_checksum ^= result_b;
    printf("Test B result: %d\n", result_b);
    
    /* Test C: Nested loops */
    int result_c = test_nested_loops(10, 100);
    final_checksum ^= result_c;
    printf("Test C result: %d\n", result_c);
    
    /* Test D: Parameter-based loop */
    int result_d = test_param_loop(750);
    final_checksum ^= result_d;
    printf("Test D result: %d\n", result_d);
    
    /* Test E: Volatile bound loop */
    int result_e = test_volatile_bound();
    final_checksum ^= result_e;
    printf("Test E result: %d\n", result_e);
    
    /* Test F: Do-while pattern */
    int result_f = test_do_while_pattern(300);
    final_checksum ^= result_f;
    printf("Test F result: %d\n", result_f);
    
    /* Add volatile sink to final output to prevent dead code elimination */
    final_checksum ^= volatile_sink;
    
    printf("Final checksum: %d\n", final_checksum);
    printf("All loops executed successfully.\n");
    
    return 0;
}
