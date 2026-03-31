/* loop-doloop-test.c
 * Test program to cover decrement-and-compare tail pattern matching
 * in GCC's loop-doloop optimization pass.
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loop bodies */
volatile int global_sink = 0;
int dummy_array[1000];

/* Non-inline function to prevent loop elimination */
__attribute__((noinline)) void dummy_side_effect(int value) {
    global_sink += value;
}

/* Function A: Basic for loop with int counter using post-decrement pattern */
__attribute__((optimize("O2")))
int test_basic_for_loop(int iterations) {
    volatile int local_sink = 0;
    int sum = 0;
    
    /* Canonical pattern: for (int i = N; i-- > 0;) */
    for (int i = iterations; i-- > 0;) {
        /* Simple side effect to prevent elimination */
        local_sink += i;
        sum += i;
        dummy_array[i & 999] = i;  /* Store to external array */
    }
    
    dummy_side_effect(local_sink);
    return sum;
}

/* Function B: While loop with unsigned int counter */
__attribute__((optimize("O2")))
int test_while_loop_unsigned(unsigned int count) {
    volatile int local_sink = 0;
    int sum = 0;
    unsigned int n = count;
    
    /* Pattern: while (n--) */
    while (n--) {
        local_sink += (int)n;
        sum += (int)n;
        dummy_side_effect((int)n);
    }
    
    return sum;
}

/* Function C: Nested loops with inner loop using the pattern */
__attribute__((optimize("O2")))
int test_nested_loops(int outer_iter, int inner_iter) {
    volatile int local_sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        int inner = inner_iter;
        
        /* Inner loop with decrement-and-compare tail */
        while (inner--) {
            local_sink += o * inner;
            total += inner;
            dummy_array[(o * inner_iter + inner) & 999] = inner;
        }
    }
    
    dummy_side_effect(local_sink);
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
__attribute__((optimize("O2")))
int test_param_counter(int count) {
    volatile int local_sink = 0;
    int sum = 0;
    int i = count;
    
    /* Using do-while to ensure tail comparison */
    if (i > 0) {
        do {
            local_sink += i;
            sum += i;
            dummy_side_effect(i);
        } while (--i > 0);  /* Decrement and compare at tail */
    }
    
    return sum;
}

/* Function E: Multiple decrement-and-compare patterns in same function */
__attribute__((optimize("O2")))
int test_multiple_loops(void) {
    volatile int local_sink = 0;
    int result = 0;
    
    /* First loop */
    for (int i = 100; i-- > 0;) {
        local_sink += i;
        result += i * 2;
    }
    
    /* Second loop with different counter */
    unsigned int j = 50;
    while (j--) {
        local_sink += (int)j;
        result -= (int)j;
        dummy_array[j & 999] = result;
    }
    
    dummy_side_effect(local_sink);
    return result;
}

/* Function F: Loop with volatile bound to prevent compile-time folding */
__attribute__((optimize("O2")))
int test_volatile_bound(void) {
    volatile int bound = 200;  /* Volatile prevents constant propagation */
    volatile int local_sink = 0;
    int sum = 0;
    int i = bound;
    
    while (i--) {
        local_sink += i;
        sum += i * 3;
        dummy_side_effect(i);
    }
    
    return sum;
}

/* Main driver that calls all test functions */
int main(void) {
    int checksum = 0;
    
    printf("Starting loop-doloop pattern tests...\n");
    
    /* Test A: Basic for loop */
    checksum += test_basic_for_loop(1000);
    printf("Test A completed\n");
    
    /* Test B: While loop with unsigned */
    checksum += test_while_loop_unsigned(500);
    printf("Test B completed\n");
    
    /* Test C: Nested loops */
    checksum += test_nested_loops(10, 100);
    printf("Test C completed\n");
    
    /* Test D: Parameter counter */
    checksum += test_param_counter(300);
    printf("Test D completed\n");
    
    /* Test E: Multiple loops */
    checksum += test_multiple_loops();
    printf("Test E completed\n");
    
    /* Test F: Volatile bound */
    checksum += test_volatile_bound();
    printf("Test F completed\n");
    
    /* Add some noise to prevent dead code elimination */
    checksum += global_sink;
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed successfully.\n");
    
    return 0;
}
