/* test_loop_doloop.c
 * This program generates loops with decrement-and-compare-tail patterns
 * to trigger coverage in GCC's loop-doloop.cc pass lines 136-150.
 */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Volatile sink to prevent optimization */
static volatile int volatile_sink = 0;

/* Non-inline dummy function to create side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    volatile_sink += value;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_basic_for_loop(int iterations) {
    int sum = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        /* Simple side effect to prevent optimization */
        sum += i;
        dummy_side_effect(i);
    }
    
    return sum;
}

/* Function B: While loop with unsigned int counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop(unsigned int count) {
    unsigned int result = 0;
    
    /* Decrement-and-compare in while condition */
    while (count--) {
        result += count;
        extern_array[count % 1000] = count;  /* External side effect */
    }
    
    return result;
}

/* Function C: Nested loops with pattern in inner loop */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop uses decrement-and-compare pattern */
        for (int i = inner; i-- > 0;) {
            total += i * o;
            volatile_sink = total;  /* Volatile store */
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    int checksum = 0;
    
    /* Counter from parameter ensures runtime value */
    for (int i = count; i-- > 0;) {
        checksum ^= i;  /* Non-trivial computation */
        dummy_side_effect(checksum);
    }
    
    return checksum;
}

/* Function E: Loop with volatile bound to prevent compile-time folding */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 1000;  /* Volatile prevents constant propagation */
    int sum = 0;
    
    int n = bound;
    while (n--) {
        sum += n * 2;
        extern_array[n % 100] = sum;  /* External side effect */
    }
    
    return sum;
}

/* Function F: Do-while loop that should also match the pattern */
int __attribute__((optimize("O2"))) test_do_while_loop(int iterations) {
    int i = iterations;
    int result = 0;
    
    if (i <= 0) return 0;
    
    do {
        result += i;
        volatile_sink = result;
    } while (--i > 0);  /* Decrement and compare at end */
    
    return result;
}

/* Function G: Loop with different integer type (short) */
int __attribute__((optimize("O2"))) test_short_loop(short count) {
    short s = count;
    int sum = 0;
    
    while (s--) {
        sum += s;
        dummy_side_effect(s);
    }
    
    return sum;
}

/* Main driver that calls all test functions */
int main(void) {
    int final_checksum = 0;
    
    printf("Testing loop-doloop pattern coverage...\n");
    
    /* Call each test function with different parameters */
    final_checksum ^= test_basic_for_loop(1000);
    printf("Basic for loop completed\n");
    
    final_checksum ^= test_while_loop(500);
    printf("While loop completed\n");
    
    final_checksum ^= test_nested_loops(10, 100);
    printf("Nested loops completed\n");
    
    final_checksum ^= test_param_loop(750);
    printf("Parameter loop completed\n");
    
    final_checksum ^= test_volatile_bound();
    printf("Volatile bound loop completed\n");
    
    final_checksum ^= test_do_while_loop(300);
    printf("Do-while loop completed\n");
    
    final_checksum ^= test_short_loop(200);
    printf("Short loop completed\n");
    
    printf("Final checksum: %d\n", final_checksum);
    printf("Volatile sink value: %d\n", volatile_sink);
    
    return 0;
}

/* Define the external array */
int extern_array[10000] = {0};
