/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Dummy function with side effects */
static void __attribute__((noinline)) dummy_side_effect(int value) {
    /* Use volatile to prevent optimization */
    static volatile int sink = 0;
    sink += value;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_decrement(int iterations) {
    volatile int sum = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sum += i;
        dummy_side_effect(i);
    }
    
    return sum;
}

/* Function B: while loop with unsigned counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int n) {
    volatile unsigned int checksum = 0;
    
    /* Decrement and compare in loop condition */
    while (n--) {
        checksum ^= n;
        extern_array[n % 1000] = n;  /* External side effect */
    }
    
    return checksum;
}

/* Function C: Nested loops with inner decrement pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop uses decrement-and-compare pattern */
        for (int i = inner; i-- > 0;) {
            total += o * i;
            dummy_side_effect(o + i);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    volatile int result = 0;
    int local_count = count;  /* Copy to local to ensure decrement happens */
    
    /* Use while loop with decrement pattern */
    while (local_count--) {
        result += (local_count * 2);
        extern_array[local_count % 500] = result;
    }
    
    return result;
}

/* Function E: Loop with volatile bound to prevent early optimization */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 1000;
    volatile int accumulator = 0;
    
    /* The volatile bound should preserve the loop structure */
    for (int i = bound; i-- > 0;) {
        accumulator += i * 3;
        dummy_side_effect(accumulator);
    }
    
    return accumulator;
}

/* Function F: Do-while loop converted from for loop */
int __attribute__((optimize("O2"))) test_do_while_pattern(int limit) {
    volatile int counter = limit;
    volatile int output = 0;
    
    if (counter > 0) {
        do {
            output += counter * 5;
            extern_array[counter % 100] = output;
        } while (counter-- > 1);  /* Decrement and compare at end */
    }
    
    return output;
}

/* Main driver that ensures all loops execute */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test different loop patterns */
    checksum += test_for_loop_decrement(100);
    printf("Test A complete\n");
    
    checksum += test_while_loop_decrement(200);
    printf("Test B complete\n");
    
    checksum += test_nested_loops(10, 50);
    printf("Test C complete\n");
    
    checksum += test_param_counter(150);
    printf("Test D complete\n");
    
    checksum += test_volatile_bound();
    printf("Test E complete\n");
    
    checksum += test_do_while_pattern(75);
    printf("Test F complete\n");
    
    printf("Final checksum: %d\n", checksum);
    printf("All loops executed successfully.\n");
    
    return 0;
}

/* Define the external array */
int extern_array[10000] = {0};
