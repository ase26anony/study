/* test_loop_doloop.c
 * This program generates loops with decrement-and-compare-tail patterns
 * to trigger coverage in GCC's loop-doloop.cc lines 136-150
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of loop bodies */
volatile int global_sink = 0;
int global_array[10000];

/* Non-inline function to prevent optimization */
__attribute__((noinline)) void dummy_operation(int value) {
    global_sink += value;
}

/* Function A: Basic for loop with int counter */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_basic_for_loop(void) {
    int sum = 0;
    volatile int local_sink = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = 1000; i-- > 0;) {
        sum += i;
        local_sink = i;  /* Volatile store prevents dead code elimination */
        global_array[i & 1023] = i;  /* External side effect */
    }
    
    dummy_operation(sum);
    return sum;
}

/* Function B: while loop with unsigned int counter */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_while_loop_unsigned(void) {
    unsigned int n = 500;
    int sum = 0;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        sum += (int)n;
        global_sink = (int)n;  /* Global volatile side effect */
        dummy_operation((int)n);
    }
    
    return sum;
}

/* Function C: Nested loops with inner loop using pattern */
__attribute__((optimize("O2", "no-unroll-loops", "no-peel-loops")))
int test_nested_loops(void) {
    int outer_sum = 0;
    volatile int outer_bound = 10;
    
    for (int outer = 0; outer < outer_bound; outer++) {
        int inner_counter = 100;
        
        /* Inner loop with decrement-and-compare pattern */
        while (inner_counter--) {
            outer_sum += inner_counter + outer;
            global_array[(inner_counter + outer) & 1023] = inner_counter;
        }
    }
    
    dummy_operation(outer_sum);
    return outer_sum;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_param_loop(int count) {
    int sum = 0;
    
    /* Use parameter to prevent compile-time folding */
    for (int i = count; i-- > 0;) {
        sum += i;
        /* Multiple side effects to prevent optimization */
        global_sink = i;
        if ((i & 1) == 0) {
            dummy_operation(i);
        }
    }
    
    return sum;
}

/* Function E: Do-while loop with explicit decrement-and-compare */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_dowhile_pattern(void) {
    int counter = 300;
    int sum = 0;
    
    if (counter <= 0) return 0;
    
    do {
        sum += counter;
        global_array[counter & 1023] = counter;
    } while (counter-- > 1);  /* Explicit decrement-and-compare */
    
    return sum;
}

/* Function F: Loop with volatile bound to prevent early optimization */
__attribute__((optimize("O2", "no-unroll-loops")))
int test_volatile_bound_loop(void) {
    volatile int volatile_bound = 400;
    int bound = volatile_bound;  /* Force runtime read */
    int sum = 0;
    
    for (int i = bound; i-- > 0;) {
        sum += i;
        /* Array access with volatile index calculation */
        int idx = i + global_sink;
        global_array[idx & 1023] = i;
    }
    
    return sum;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern coverage...\n");
    
    /* Execute all test functions */
    checksum += test_basic_for_loop();
    printf("Basic for loop completed\n");
    
    checksum += test_while_loop_unsigned();
    printf("While loop with unsigned completed\n");
    
    checksum += test_nested_loops();
    printf("Nested loops completed\n");
    
    checksum += test_param_loop(200);
    printf("Parameter loop completed\n");
    
    checksum += test_dowhile_pattern();
    printf("Do-while pattern completed\n");
    
    checksum += test_volatile_bound_loop();
    printf("Volatile bound loop completed\n");
    
    /* Add some noise to prevent optimization of checksum */
    checksum += global_sink;
    checksum += global_array[0];
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum is non-zero, loops executed successfully.\n");
    
    return 0;
}
