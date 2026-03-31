/* test_loop_doloop.c
 * This program generates loops that should produce the RTL pattern:
 * (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * to cover the uncovered lines in loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Dummy function with side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    /* Use volatile to ensure side effect isn't optimized away */
    static volatile int sink = 0;
    sink += value;
}

/* Function A: Basic for loop with int counter using decrement-and-compare pattern */
int __attribute__((optimize("O2"))) test_basic_for_loop(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Classic decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sink = i;  /* Volatile store prevents optimization */
        sum += i;
        dummy_side_effect(i);  /* External call prevents loop removal */
    }
    
    return sum;
}

/* Function B: While loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop(unsigned int count) {
    volatile int sink = 0;
    int sum = 0;
    unsigned int n = count;
    
    /* while(n--) pattern - should generate decrement-and-compare */
    while (n--) {
        sink = (int)n;
        sum += (int)n;
        /* Store to external array to ensure side effect */
        if (n < 10000) extern_array[n] = (int)n;
    }
    
    return sum;
}

/* Function C: Nested loops with inner loop using the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare pattern */
        for (int i = inner; i-- > 0;) {
            sink = o * 1000 + i;
            total += i;
            dummy_side_effect(i);
        }
    }
    
    return total;
}

/* Function D: Loop with counter as function parameter (non-constant) */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Use volatile to hide loop bound from constant propagation */
    volatile int hidden_count = count;
    
    /* Loop with parameter counter - prevents compile-time folding */
    for (int i = hidden_count; i-- > 0;) {
        sink = i;
        sum += i;
        if (i < 10000) extern_array[i] = i;
    }
    
    return sum;
}

/* Function E: Do-while loop that should also match the pattern */
int __attribute__((optimize("O2"))) test_dowhile_loop(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    int i = iterations;
    
    if (i <= 0) return 0;
    
    do {
        sink = i;
        sum += i;
        dummy_side_effect(i);
    } while (--i > 0);  /* Decrement and compare at the end */
    
    return sum;
}

/* Function F: Loop with different data type (short) */
int __attribute__((optimize("O2"))) test_short_loop(short count) {
    volatile short sink = 0;
    int sum = 0;
    
    for (short i = count; i-- > 0;) {
        sink = i;
        sum += i;
        dummy_side_effect(i);
    }
    
    return sum;
}

/* Global array definition */
int extern_array[10000] = {0};

/* Main driver that calls all test functions */
int main(void) {
    int checksum = 0;
    
    printf("Testing loops for loop-doloop pass coverage...\n");
    
    /* Test with various iteration counts to explore different paths */
    checksum += test_basic_for_loop(1000);
    printf("Basic for loop completed\n");
    
    checksum += test_while_loop(500);
    printf("While loop completed\n");
    
    checksum += test_nested_loops(10, 100);
    printf("Nested loops completed\n");
    
    checksum += test_param_loop(750);
    printf("Parameter loop completed\n");
    
    checksum += test_dowhile_loop(300);
    printf("Do-while loop completed\n");
    
    checksum += test_short_loop(200);
    printf("Short loop completed\n");
    
    /* Verify array was written to */
    int array_sum = 0;
    for (int i = 0; i < 1000; i++) {
        array_sum += extern_array[i];
    }
    checksum += array_sum;
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum is non-zero, loops executed successfully.\n");
    
    return 0;
}
