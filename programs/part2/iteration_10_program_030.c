/* test_loop_doloop.c
 * 
 * This program is designed to exercise the loop-doloop pass in GCC,
 * specifically targeting the RTL pattern matching for decrement-and-compare
 * loop tails: (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * 
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 test_loop_doloop.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Dummy function with side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    /* Use volatile to ensure the call isn't optimized away */
    volatile static int sink = 0;
    sink += value;
}

/* Volatile variable for loop bodies */
volatile int volatile_sink = 0;

/* Global variable to hide loop bounds from constant propagation */
int global_bound = 1000;

/* Function A: Basic for loop with int counter, decrement-and-compare pattern */
int __attribute__((optimize("O2"))) test_for_loop_decrement(void) {
    int sum = 0;
    volatile int local_sink = 0;
    
    /* Classic decrement-and-compare pattern: for (int i = N; i-- > 0;) */
    for (int i = 1000; i-- > 0;) {
        /* Simple side effect to prevent loop removal */
        local_sink += i;
        sum += i;
    }
    
    /* Use the volatile result to prevent dead code elimination */
    volatile_sink += local_sink;
    return sum;
}

/* Function B: while loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop_decrement(void) {
    unsigned int n = 500;
    int sum = 0;
    
    /* while (n--) pattern - should generate decrement-and-compare */
    while (n--) {
        /* Store to external array to create side effect */
        extern_array[n] = (int)n;
        sum += (int)n;
        dummy_side_effect(n);
    }
    
    return sum;
}

/* Function C: Nested loops with inner loop using decrement pattern */
int __attribute__((optimize("O2"))) test_nested_loops(void) {
    int outer_sum = 0;
    volatile int inner_sink = 0;
    
    for (int outer = 10; outer > 0; outer--) {
        /* Inner loop uses decrement-and-compare pattern */
        int inner = 100;
        while (inner--) {
            /* Multiple side effects to prevent optimization */
            inner_sink += inner * outer;
            outer_sum += inner;
            dummy_side_effect(inner);
        }
    }
    
    volatile_sink += inner_sink;
    return outer_sum;
}

/* Function D: Loop with parameter counter to prevent constant propagation */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    int sum = 0;
    
    /* Use parameter as loop bound - compiler can't assume constant value */
    for (int i = count; i-- > 0;) {
        /* Array access with volatile-like behavior */
        sum += i;
        if (i < 10000) {
            extern_array[i] = i;
        }
    }
    
    return sum;
}

/* Function E: Loop with volatile-bound to prevent compile-time folding */
int __attribute__((optimize("O2"))) test_volatile_bound_loop(void) {
    volatile int volatile_bound = 200;
    int bound = volatile_bound;  /* Read volatile to get runtime value */
    int sum = 0;
    
    /* Decrement-and-compare with runtime-determined bound */
    for (int i = bound; i-- > 0;) {
        sum += i;
        volatile_sink += i;
    }
    
    return sum;
}

/* Function F: Mixed decrement patterns to test edge cases */
int __attribute__((optimize("O2"))) test_mixed_patterns(void) {
    int sum = 0;
    
    /* Pattern 1: Classic for loop with post-decrement in condition */
    for (int i = 300; i-- > 0;) {
        sum += i * 2;
    }
    
    /* Pattern 2: do-while with decrement-and-compare */
    int j = 200;
    do {
        sum += j;
        dummy_side_effect(j);
    } while (j-- > 0);
    
    return sum;
}

/* Main function that drives all tests and verifies correctness */
int main(void) {
    int total_checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test 1: Basic for loop with decrement-and-compare */
    int result1 = test_for_loop_decrement();
    total_checksum += result1;
    printf("Test 1 result: %d (expected: %d)\n", result1, 1000*999/2);
    
    /* Test 2: while loop with unsigned decrement */
    int result2 = test_while_loop_decrement();
    total_checksum += result2;
    printf("Test 2 result: %d (expected: %d)\n", result2, 500*499/2);
    
    /* Test 3: Nested loops */
    int result3 = test_nested_loops();
    total_checksum += result3;
    printf("Test 3 result: %d (expected: %d)\n", result3, 10 * (100*99/2));
    
    /* Test 4: Parameterized loop */
    int result4 = test_param_loop(400);
    total_checksum += result4;
    printf("Test 4 result: %d (expected: %d)\n", result4, 400*399/2);
    
    /* Test 5: Volatile bound loop */
    int result5 = test_volatile_bound_loop();
    total_checksum += result5;
    printf("Test 5 result: %d (expected: %d)\n", result5, 200*199/2);
    
    /* Test 6: Mixed patterns */
    int result6 = test_mixed_patterns();
    total_checksum += result6;
    printf("Test 6 result: %d\n", result6);
    
    /* Final checksum */
    printf("\nTotal checksum: %d\n", total_checksum);
    printf("All loops executed successfully.\n");
    
    /* Use the results to prevent dead code elimination */
    if (total_checksum > 0) {
        return 0;
    } else {
        return 1;
    }
}

/* Define the external array */
int extern_array[10000] = {0};
