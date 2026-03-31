/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */
#include <stdio.h>
#include <stdint.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Volatile sink to prevent optimization */
static volatile int volatile_sink = 0;

/* Non-inline dummy function with side effect */
__attribute__((noinline)) static void dummy_side_effect(int value) {
    volatile_sink += value;
}

/* Prevent constant propagation of loop bounds */
static volatile int volatile_bound = 1000;

/* Function A: Basic for loop with int counter */
__attribute__((optimize("O2", "no-unroll-loops")))
static int test_basic_for_loop(void) {
    int sum = 0;
    /* Canonical decrement-and-compare pattern */
    for (int i = 1000; i-- > 0;) {
        /* Simple side effect to prevent loop removal */
        sum += i;
        extern_array[i & 1023] = i;  /* Mask to avoid out-of-bounds */
    }
    return sum;
}

/* Function B: While loop with unsigned counter */
__attribute__((optimize("O2", "no-unroll-loops")))
static int test_while_loop_unsigned(void) {
    unsigned int n = 500;
    int sum = 0;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        sum += (int)n;
        dummy_side_effect(n);
    }
    return sum;
}

/* Function C: Nested loops with pattern in inner loop */
__attribute__((optimize("O2", "no-unroll-loops")))
static int test_nested_loops(void) {
    int outer_sum = 0;
    
    for (int outer = 10; outer-- > 0;) {
        int inner_sum = 0;
        /* Inner loop with the target pattern */
        for (int inner = 100; inner-- > 0;) {
            inner_sum += inner;
            volatile_sink += outer * inner;
        }
        outer_sum += inner_sum;
    }
    return outer_sum;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
__attribute__((optimize("O2", "no-unroll-loops")))
static int test_param_loop(int count) {
    int sum = 0;
    
    /* Use parameter as loop bound */
    for (int i = count; i-- > 0;) {
        sum += i;
        /* Use volatile to prevent reordering */
        int temp = volatile_sink;
        extern_array[i & 1023] = temp + i;
    }
    return sum;
}

/* Function E: Loop with volatile bound */
__attribute__((optimize("O2", "no-unroll-loops")))
static int test_volatile_bound_loop(void) {
    int bound = volatile_bound;  /* Read from volatile */
    int sum = 0;
    
    for (int i = bound; i-- > 0;) {
        sum += i;
        dummy_side_effect(i);
    }
    return sum;
}

/* Function F: Do-while loop (already in canonical form) */
__attribute__((optimize("O2", "no-unroll-loops")))
static int test_do_while_loop(void) {
    int i = 200;
    int sum = 0;
    
    do {
        sum += i;
        extern_array[i & 1023] = sum;
    } while (i-- > 0);
    
    return sum;
}

/* Function G: Loop with different integer type (short) */
__attribute__((optimize("O2", "no-unroll-loops")))
static int test_short_counter(void) {
    short counter = 300;
    int sum = 0;
    
    while (counter--) {
        sum += counter;
        volatile_sink += counter;
    }
    return sum;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Starting loop-doloop pattern tests...\n");
    
    /* Run all test functions and accumulate checksum */
    checksum += test_basic_for_loop();
    checksum += test_while_loop_unsigned();
    checksum += test_nested_loops();
    checksum += test_param_loop(400);
    checksum += test_volatile_bound_loop();
    checksum += test_do_while_loop();
    checksum += test_short_counter();
    
    /* Add volatile sink to final checksum to ensure all side effects are used */
    checksum += volatile_sink;
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}

/* Define the external array */
int extern_array[10000] = {0};
