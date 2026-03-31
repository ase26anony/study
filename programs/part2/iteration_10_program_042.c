/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Dummy function with side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    /* Use volatile to prevent optimization */
    static volatile int sink = 0;
    sink += value;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_basic_for_loop(void) {
    volatile int result = 0;
    
    /* Canonical decrement-and-compare pattern */
    for (int i = 1000; i-- > 0;) {
        result += i;
        dummy_side_effect(i);
    }
    
    return result;
}

/* Function B: While loop with unsigned counter */
int __attribute__((optimize("O2"))) test_while_loop(void) {
    volatile int result = 0;
    unsigned int n = 500;
    
    /* Another decrement-and-compare pattern */
    while (n--) {
        result += (int)n;
        extern_array[n % 1000] = (int)n;  /* External side effect */
    }
    
    return result;
}

/* Function C: Nested loops with inner loop using pattern */
int __attribute__((optimize("O2"))) test_nested_loops(void) {
    volatile int result = 0;
    int outer_count = 10;
    
    for (int j = 0; j < outer_count; j++) {
        /* Inner loop with decrement-and-compare */
        int inner = 100;
        while (inner--) {
            result += j * inner;
            dummy_side_effect(j + inner);
        }
    }
    
    return result;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    volatile int result = 0;
    
    /* Use parameter to prevent compile-time folding */
    int i = count;
    while (i--) {
        result += i;
        extern_array[i % 1000] = result;  /* External side effect */
    }
    
    return result;
}

/* Function E: Loop with volatile bound (prevents early optimization) */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 300;
    volatile int result = 0;
    
    /* Volatile bound prevents constant propagation */
    int i = bound;
    do {
        result += i;
        dummy_side_effect(i);
    } while (i-- > 0);
    
    return result;
}

/* Function F: Multiple decrement-and-compare loops in same function */
int __attribute__((optimize("O2"))) test_multiple_loops(void) {
    volatile int result = 0;
    
    /* First loop */
    for (int i = 200; i-- > 0;) {
        result += i * 2;
    }
    
    /* Second loop with different counter */
    unsigned int j = 150;
    while (j--) {
        result -= (int)j;
        extern_array[j % 1000] = result;
    }
    
    return result;
}

/* Function G: Loop with pre-decrement pattern */
int __attribute__((optimize("O2"))) test_predecrement_loop(void) {
    volatile int result = 0;
    int count = 400;
    
    /* Alternative pattern that should also generate decrement-and-compare */
    while (--count >= 0) {
        result += count;
        dummy_side_effect(count);
    }
    
    return result;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Call all test functions and accumulate checksum */
    checksum += test_basic_for_loop();
    printf("Basic for loop completed\n");
    
    checksum += test_while_loop();
    printf("While loop completed\n");
    
    checksum += test_nested_loops();
    printf("Nested loops completed\n");
    
    checksum += test_param_loop(250);
    printf("Parameter loop completed\n");
    
    checksum += test_volatile_bound();
    printf("Volatile bound loop completed\n");
    
    checksum += test_multiple_loops();
    printf("Multiple loops completed\n");
    
    checksum += test_predecrement_loop();
    printf("Pre-decrement loop completed\n");
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify expected value (sum of arithmetic series) */
    int expected = 499500;  /* sum(0..999) from test_basic_for_loop */
    expected += 124750;     /* sum(0..499) from test_while_loop */
    expected += 49500;      /* from test_nested_loops */
    expected += 31125;      /* sum(0..249) from test_param_loop */
    expected += 45150;      /* sum(0..300) from test_volatile_bound */
    expected += 39800;      /* from test_multiple_loops */
    expected += 79800;      /* sum(0..399) from test_predecrement_loop */
    
    printf("Expected checksum: %d\n", expected);
    
    if (checksum == expected) {
        printf("SUCCESS: All loops executed correctly\n");
        return 0;
    } else {
        printf("ERROR: Checksum mismatch\n");
        return 1;
    }
}

/* Define the external array */
int extern_array[10000] = {0};
