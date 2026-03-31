/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of loop bodies */
volatile int global_sink = 0;
extern int external_array[1000];

/* Dummy function with side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    global_sink += value;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_int(int iterations) {
    volatile int local_sink = 0;
    int sum = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        local_sink += i;           /* Volatile side effect */
        sum += i & 0xFF;           /* Prevent dead code elimination */
        dummy_side_effect(i);      /* External side effect */
    }
    
    return sum;
}

/* Function B: While loop with unsigned int counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int n) {
    volatile unsigned int local_sink = 0;
    unsigned int checksum = 0;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        local_sink += n;
        checksum ^= n;             /* Non-linear operation */
        external_array[n % 1000] = n; /* External memory store */
    }
    
    return checksum;
}

/* Function C: Nested loops with inner decrement pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare tail */
        for (int i = inner; i-- > 0;) {
            sink += o * i;
            total += (o * 1000) + i;
            dummy_side_effect(i);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    volatile int accumulator = 0;
    int result = 0;
    
    /* Counter from parameter ensures runtime value */
    while (count--) {
        accumulator += count;
        result = (result * 31 + count) & 0xFFFF;
        external_array[count % 1000] = result;
    }
    
    return result;
}

/* Function E: Loop with volatile bound (prevents compile-time folding) */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 250;      /* Volatile prevents constant propagation */
    int counter = bound;
    int sum = 0;
    
    /* Decrement pattern with volatile-influenced bound */
    while (counter--) {
        sum += counter;
        global_sink = counter;     /* Global volatile side effect */
    }
    
    return sum;
}

/* Function F: Do-while converted from for loop (should still match pattern) */
int __attribute__((optimize("O2"))) test_do_while_style(int limit) {
    int i = limit;
    int checksum = 0;
    
    if (i <= 0) return 0;
    
    do {
        checksum += i;
        dummy_side_effect(i);
    } while (i-- > 1);            /* Decrement and compare at tail */
    
    return checksum;
}

/* Main driver that verifies all loops execute correctly */
int main(void) {
    int total_checksum = 0;
    
    /* Initialize external array */
    for (int i = 0; i < 1000; i++) {
        external_array[i] = 0;
    }
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test each function with different parameters */
    total_checksum += test_for_loop_int(1000);
    total_checksum += test_while_loop_unsigned(500);
    total_checksum += test_nested_loops(10, 100);
    total_checksum += test_param_counter(750);
    total_checksum += test_volatile_bound();
    total_checksum += test_do_while_style(300);
    
    /* Add global sink to prevent elimination */
    total_checksum += global_sink;
    
    /* Verify array was touched */
    for (int i = 0; i < 1000; i++) {
        total_checksum += external_array[i];
    }
    
    printf("Final checksum: %d\n", total_checksum);
    printf("If checksum != 0, loops executed and produced side effects.\n");
    
    return 0;
}

/* External array definition */
int external_array[1000];
