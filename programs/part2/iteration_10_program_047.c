/* test_loop_doloop.c
 * 
 * This program is designed to trigger coverage of the decrement-and-compare
 * tail pattern matching logic in GCC's loop-doloop pass (loop-doloop.cc).
 * Each test function contains a loop that should compile to the RTL pattern:
 *   (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * The loops use volatile variables and external side effects to prevent
 * premature optimization and ensure the pattern reaches the loop-doloop pass.
 */

#include <stdio.h>
#include <stdlib.h>

/* External array to create side effects */
extern int extern_array[10000];

/* Dummy function with noinline to prevent inlining */
void __attribute__((noinline)) dummy_side_effect(int value) {
    /* Use volatile to ensure the call isn't optimized away */
    volatile static int sink = 0;
    sink += value;
}

/* Function A: Basic for loop with int counter */
int test_basic_for_loop(void) {
    volatile int result = 0;
    /* The canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = 1000; i-- > 0;) {
        /* Use volatile to prevent loop body removal */
        volatile int sink = i;
        result += sink;
        dummy_side_effect(i);
    }
    return result;
}

/* Function B: while loop with unsigned int counter */
int test_while_loop_unsigned(void) {
    volatile int result = 0;
    unsigned int n = 500;
    /* while (n--) pattern - should generate decrement-and-compare */
    while (n--) {
        volatile int sink = n;
        result += sink;
        /* Store to external array to create side effect */
        extern_array[n % 1000] = n;
    }
    return result;
}

/* Function C: Nested loops with inner loop using the pattern */
int test_nested_loops(void) {
    volatile int result = 0;
    int outer_count = 10;
    
    for (int j = 0; j < outer_count; j++) {
        /* Inner loop uses the decrement-and-compare pattern */
        int inner_count = 100;
        while (inner_count--) {
            volatile int sink = inner_count + j;
            result += sink;
            dummy_side_effect(sink);
        }
    }
    return result;
}

/* Function D: Loop with counter as function parameter */
int test_param_loop(int count) {
    volatile int result = 0;
    /* Use parameter to prevent constant propagation */
    int i = count;
    
    /* Decrement-and-compare pattern with parameterized count */
    while (i--) {
        volatile int sink = i;
        result += sink;
        extern_array[i % 1000] = i;
    }
    return result;
}

/* Function E: Loop with volatile bound to prevent compile-time folding */
int test_volatile_bound_loop(void) {
    volatile int bound = 300;
    volatile int result = 0;
    int i = bound;
    
    /* The volatile bound ensures the decrement isn't optimized away */
    for (; i-- > 0;) {
        result += i;
        dummy_side_effect(i);
    }
    return result;
}

/* Function F: Do-while loop that naturally checks condition at end */
int test_dowhile_loop(void) {
    volatile int result = 0;
    int i = 200;
    
    /* do-while with decrement-and-compare */
    if (i > 0) {
        do {
            result += i;
            extern_array[i % 1000] = i;
        } while (i-- > 1);  /* Note: careful with boundary to avoid underflow */
    }
    return result;
}

/* Main driver that runs all tests and computes checksum */
int main(void) {
    int checksum = 0;
    
    printf("Running loop pattern tests for loop-doloop pass coverage...\n");
    
    /* Run each test and accumulate results */
    checksum += test_basic_for_loop();
    checksum += test_while_loop_unsigned();
    checksum += test_nested_loops();
    checksum += test_param_loop(400);
    checksum += test_volatile_bound_loop();
    checksum += test_dowhile_loop();
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum is non-zero, loops executed successfully.\n");
    
    return 0;
}

/* Define the external array */
int extern_array[10000] = {0};
