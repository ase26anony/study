/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of loops */
volatile int global_counter = 0;
volatile int sink;

/* Non-inline function to prevent loop elimination */
__attribute__((noinline)) void dummy_side_effect(int value) {
    sink = value;
}

/* External array to force memory operations */
extern int extern_array[1000];

/* Function A: Basic for loop with int counter */
__attribute__((optimize("O2"))) 
int test_basic_for_loop(int iterations) {
    volatile int local_sink = 0;
    int sum = 0;
    
    /* Canonical decrement-and-compare pattern */
    for (int i = iterations; i-- > 0;) {
        sum += i;
        local_sink = i;  /* Volatile side effect */
        dummy_side_effect(i);
    }
    
    return sum;
}

/* Function B: While loop with unsigned counter */
__attribute__((optimize("O2")))
int test_while_loop(unsigned int count) {
    volatile int local_sink = 0;
    int sum = 0;
    unsigned int n = count;
    
    /* Another decrement-and-compare pattern */
    while (n--) {
        sum += (int)n;
        local_sink = (int)n;
        extern_array[n % 1000] = (int)n;  /* External memory side effect */
    }
    
    return sum;
}

/* Function C: Nested loops with inner decrement pattern */
__attribute__((optimize("O2")))
int test_nested_loops(int outer_iter, int inner_iter) {
    volatile int local_sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        int inner = inner_iter;
        
        /* Inner loop with decrement-and-compare */
        while (inner--) {
            total += o * inner;
            local_sink = inner;
            dummy_side_effect(inner);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
__attribute__((optimize("O2")))
int test_param_loop(int count) {
    volatile int local_sink = 0;
    int sum = 0;
    int i = count;
    
    /* Decrement pattern with parameter */
    do {
        sum += i;
        local_sink = i;
        extern_array[i % 1000] = i;
    } while (i-- > 0);
    
    return sum;
}

/* Function E: Multiple decrement patterns in same function */
__attribute__((optimize("O2")))
int test_multiple_loops(void) {
    volatile int local_sink = 0;
    int result = 0;
    
    /* First loop */
    for (int i = 100; i-- > 0;) {
        result += i;
        local_sink = i;
    }
    
    /* Second loop with different counter type */
    unsigned int j = 50;
    while (j--) {
        result -= (int)j;
        dummy_side_effect((int)j);
    }
    
    return result;
}

/* Function F: Loop with volatile bound (prevents compile-time evaluation) */
__attribute__((optimize("O2")))
int test_volatile_bound(void) {
    volatile int bound = 200;
    volatile int local_sink = 0;
    int sum = 0;
    int i = bound;
    
    /* Volatile prevents constant folding */
    while (i--) {
        sum += i;
        local_sink = i;
        global_counter++;  /* Global volatile side effect */
    }
    
    return sum;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Initialize external array */
    for (int i = 0; i < 1000; i++) {
        extern_array[i] = 0;
    }
    
    printf("Running loop tests for GCC loop-doloop pass coverage...\n");
    
    /* Test different loop patterns */
    checksum += test_basic_for_loop(100);
    printf("Test A complete\n");
    
    checksum += test_while_loop(75);
    printf("Test B complete\n");
    
    checksum += test_nested_loops(10, 20);
    printf("Test C complete\n");
    
    checksum += test_param_loop(50);
    printf("Test D complete\n");
    
    checksum += test_multiple_loops();
    printf("Test E complete\n");
    
    checksum += test_volatile_bound();
    printf("Test F complete\n");
    
    printf("Final checksum: %d\n", checksum);
    printf("Expected checksum: 107325\n");
    
    /* Verification */
    if (checksum == 107325) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Test verification failed!\n");
        return 1;
    }
}

/* Define the external array */
int extern_array[1000];
