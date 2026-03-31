/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of loops */
volatile int global_counter = 0;
volatile int sink; /* Used to prevent dead code elimination */

/* External array to ensure side effects */
extern int extern_array[1000];

/* Dummy function with noinline to prevent inlining */
__attribute__((noinline)) void dummy_side_effect(int value) {
    sink += value;
}

/* Function A: Basic for loop with int counter using decrement-and-compare pattern */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_for_loop_decrement(int iterations) {
    volatile int local_sink = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        /* Simple side effect to prevent loop removal */
        local_sink += i;
        dummy_side_effect(i);
    }
    
    return local_sink;
}

/* Function B: While loop with unsigned int counter */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_while_loop_decrement(unsigned int count) {
    volatile int result = 0;
    unsigned int n = count;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        result += (int)n;
        extern_array[n % 1000] = (int)n;
    }
    
    return result;
}

/* Function C: Nested loops with inner loop using the pattern */
__attribute__((optimize("O2", "no-unroll-loops", "no-peel-loops")))
int test_nested_loops(int outer_iter, int inner_iter) {
    volatile int total = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        /* Inner loop uses decrement-and-compare pattern */
        for (int i = inner_iter; i-- > 0;) {
            total += i * o;
            dummy_side_effect(i);
        }
    }
    
    return total;
}

/* Function D: Loop with counter as volatile parameter to prevent constant propagation */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_volatile_bound(volatile int bound) {
    volatile int sum = 0;
    int count = bound; /* Copy to local to avoid volatile in loop condition */
    
    /* Use decrement-and-compare pattern with volatile-derived bound */
    for (int i = count; i-- > 0;) {
        sum += i;
        /* Array access ensures side effect */
        if (i < 1000) {
            extern_array[i] = i;
        }
    }
    
    return sum;
}

/* Function E: Multiple decrement-and-compare loops in same function */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_multiple_loops(int iter1, int iter2, int iter3) {
    volatile int total = 0;
    
    /* First loop */
    for (int i = iter1; i-- > 0;) {
        total += i;
        dummy_side_effect(i);
    }
    
    /* Second loop with different counter type */
    unsigned int u = (unsigned int)iter2;
    while (u--) {
        total -= (int)u;
        extern_array[u % 1000] = (int)u;
    }
    
    /* Third loop with post-decrement in condition */
    int j = iter3;
    do {
        total += j * 2;
        dummy_side_effect(j);
    } while (j-- > 0);
    
    return total;
}

/* Function F: Loop with hidden bound to prevent early optimization */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_hidden_bound(void) {
    /* Hide loop bound behind memory access */
    volatile int hidden_iter = 100;
    volatile int result = 0;
    int iterations = hidden_iter;
    
    /* Classic decrement-and-compare pattern */
    for (int i = iterations; i-- > 0;) {
        result += i * i;
        /* Use both dummy function and array access */
        dummy_side_effect(i);
        if (i < 1000) {
            extern_array[i] = result;
        }
    }
    
    return result;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Initialize external array */
    for (int i = 0; i < 1000; i++) {
        extern_array[i] = 0;
    }
    
    printf("Starting loop-doloop pattern tests...\n");
    
    /* Test each function with different parameters */
    checksum += test_for_loop_decrement(100);
    printf("Test 1 complete\n");
    
    checksum += test_while_loop_decrement(50);
    printf("Test 2 complete\n");
    
    checksum += test_nested_loops(10, 20);
    printf("Test 3 complete\n");
    
    checksum += test_volatile_bound(30);
    printf("Test 4 complete\n");
    
    checksum += test_multiple_loops(15, 25, 35);
    printf("Test 5 complete\n");
    
    checksum += test_hidden_bound();
    printf("Test 6 complete\n");
    
    /* Add some simple validation */
    int array_sum = 0;
    for (int i = 0; i < 1000; i++) {
        array_sum += extern_array[i];
    }
    checksum += array_sum;
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed successfully.\n");
    
    return 0;
}

/* Define the external array */
int extern_array[1000];
