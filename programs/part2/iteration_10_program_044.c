/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdint.h>

/* External array to prevent dead code elimination */
volatile int extern_array[10000];
volatile int extern_index = 0;

/* Dummy function with side effects to prevent loop elimination */
__attribute__((noinline)) void dummy_side_effect(int value) {
    extern_array[extern_index++ % 10000] = value;
}

/* Prevent constant propagation of loop bounds */
volatile int volatile_bound = 1000;

/* Function A: Basic for loop with int counter, decrement-and-compare pattern */
__attribute__((optimize("O2"))) 
int test_basic_for_loop(void) {
    int sum = 0;
    volatile int sink = 0;
    
    /* Classic decrement-and-compare pattern: i-- > 0 */
    for (int i = 1000; i-- > 0;) {
        sink += i;           /* Volatile operation prevents elimination */
        sum += i & 0xFF;     /* Simple computation for verification */
        dummy_side_effect(i); /* External side effect */
    }
    return sum;
}

/* Function B: While loop with unsigned int counter */
__attribute__((optimize("O2"))) 
int test_while_loop_unsigned(void) {
    unsigned int n = 500;
    int sum = 0;
    volatile int sink = 0;
    
    /* while(n--) pattern - should generate decrement-and-compare */
    while (n--) {
        sink += n;
        sum += (int)n * 2;
        dummy_side_effect((int)n);
    }
    return sum;
}

/* Function C: Nested loops with inner loop using the pattern */
__attribute__((optimize("O2"))) 
int test_nested_loops(void) {
    int outer_sum = 0;
    volatile int sink = 0;
    
    for (int outer = 10; outer > 0; outer--) {
        /* Inner loop with decrement-and-compare pattern */
        int inner = 100;
        while (inner--) {
            sink += outer * inner;
            outer_sum += outer + inner;
            dummy_side_effect(inner);
        }
    }
    return outer_sum;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
__attribute__((optimize("O2"))) 
int test_param_loop(int count) {
    int sum = 0;
    volatile int sink = 0;
    
    /* Use parameter as loop bound - compiler can't assume value */
    for (int i = count; i-- > 0;) {
        sink += i * count;
        sum += i;
        dummy_side_effect(i);
        
        /* Prevent loop unrolling with conditional that's always false */
        if (count < 0) break; /* Never taken, but prevents analysis */
    }
    return sum;
}

/* Function E: Loop with volatile bound to prevent compile-time evaluation */
__attribute__((optimize("O2"))) 
int test_volatile_bound_loop(void) {
    int sum = 0;
    volatile int sink = 0;
    int bound = volatile_bound; /* Read from volatile to get dynamic value */
    
    /* Loop with bound from volatile variable */
    for (int i = bound; i-- > 0;) {
        sink += i;
        sum += i * 3;
        dummy_side_effect(i);
    }
    return sum;
}

/* Function F: Do-while loop that should also match the pattern */
__attribute__((optimize("O2"))) 
int test_dowhile_loop(void) {
    int i = 200;
    int sum = 0;
    volatile int sink = 0;
    
    /* do-while with post-decrement in condition */
    do {
        sink += i;
        sum += i * 4;
        dummy_side_effect(i);
    } while (i-- > 0);
    
    return sum;
}

/* Main driver that runs all tests and computes checksum */
int main(void) {
    int total_checksum = 0;
    
    printf("Running loop-doloop pattern tests...\n");
    
    /* Run each test and accumulate checksum */
    total_checksum += test_basic_for_loop();
    printf("Test 1 complete\n");
    
    total_checksum += test_while_loop_unsigned();
    printf("Test 2 complete\n");
    
    total_checksum += test_nested_loops();
    printf("Test 3 complete\n");
    
    total_checksum += test_param_loop(300);
    printf("Test 4 complete\n");
    
    total_checksum += test_volatile_bound_loop();
    printf("Test 5 complete\n");
    
    total_checksum += test_dowhile_loop();
    printf("Test 6 complete\n");
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Verify by also printing a value from the external array */
    printf("Sample from extern_array[0]: %d\n", extern_array[0]);
    
    return 0;
}
