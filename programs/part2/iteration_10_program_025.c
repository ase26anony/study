/* test_loop_doloop.c
 * This program generates loops with decrement-and-compare patterns
 * to trigger coverage in GCC's loop-doloop.cc pass.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of loop bodies */
volatile int global_sink = 0;
int dummy_array[1000];

/* Non-inlinable function to prevent loop elimination */
__attribute__((noinline)) void side_effect(int value) {
    global_sink += value;
}

/* Function A: Basic for loop with int counter */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_basic_for_loop(int iterations) {
    volatile int local_sink = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        /* Simple side effect to prevent elimination */
        local_sink += i;
        dummy_array[i & 999] = i;
    }
    
    return local_sink;
}

/* Function B: While loop with unsigned counter */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_while_loop(unsigned int count) {
    volatile int local_sink = 0;
    unsigned int n = count;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        local_sink += (int)n;
        side_effect(n);
    }
    
    return local_sink;
}

/* Function C: Nested loops with inner decrement pattern */
__attribute__((optimize("O2", "no-unroll-loops", "no-peel-loops")))
int test_nested_loops(int outer, int inner) {
    volatile int local_sink = 0;
    
    for (int j = 0; j < outer; j++) {
        /* Inner loop uses decrement-and-compare pattern */
        for (int i = inner; i-- > 0;) {
            local_sink += i * j;
            dummy_array[(i + j) & 999] = i;
        }
    }
    
    return local_sink;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_param_loop(int count) {
    volatile int local_sink = 0;
    int n = count;
    
    /* Use parameter to prevent compile-time folding */
    while (n--) {
        local_sink += n;
        /* Use volatile to ensure side effect */
        global_sink = n;
    }
    
    return local_sink;
}

/* Function E: Do-while with explicit decrement-and-compare */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_dowhile_loop(int iterations) {
    volatile int local_sink = 0;
    int i = iterations;
    
    if (i > 0) {
        do {
            local_sink += i;
            dummy_array[i & 999] = i;
        } while (--i > 0);  /* Decrement before compare */
    }
    
    return local_sink;
}

/* Function F: Loop with volatile bound to prevent optimization */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_volatile_bound(void) {
    volatile int bound = 100;
    volatile int local_sink = 0;
    int i = bound;
    
    /* Volatile bound prevents constant propagation */
    while (i--) {
        local_sink += i;
        side_effect(i);
    }
    
    return local_sink;
}

/* Main driver that exercises all test functions */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern coverage...\n");
    
    /* Test with different iteration counts to explore various paths */
    checksum += test_basic_for_loop(100);
    printf("Basic for loop completed\n");
    
    checksum += test_while_loop(200);
    printf("While loop completed\n");
    
    checksum += test_nested_loops(10, 50);
    printf("Nested loops completed\n");
    
    checksum += test_param_loop(150);
    printf("Parameter loop completed\n");
    
    checksum += test_dowhile_loop(75);
    printf("Do-while loop completed\n");
    
    checksum += test_volatile_bound();
    printf("Volatile bound loop completed\n");
    
    /* Verify all loops executed by checking global side effects */
    printf("Final checksum: %d\n", checksum);
    printf("Global sink value: %d\n", global_sink);
    
    /* Quick sanity check on array */
    int array_sum = 0;
    for (int i = 0; i < 1000; i++) {
        array_sum += dummy_array[i];
    }
    printf("Array checksum: %d\n", array_sum);
    
    return 0;
}
