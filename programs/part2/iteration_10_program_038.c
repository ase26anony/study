/* test_loop_doloop.c
 * 
 * This program is designed to exercise GCC's loop-doloop optimization pass
 * by generating loops with the specific decrement-and-compare pattern:
 *   (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * 
 * The loops use volatile variables and external side effects to prevent
 * premature optimization, ensuring the pattern reaches the RTL level.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent dead code elimination */
volatile int global_sink = 0;

/* External array to create side effects */
extern int extern_array[10000];

/* Non-inlinable dummy function to create side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    global_sink += value;
}

/* Function A: Basic for loop with int counter */
int test_basic_for_loop(int iterations) {
    volatile int local_sink = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        local_sink += i;
        dummy_side_effect(i);
    }
    
    return local_sink;
}

/* Function B: While loop with unsigned int counter */
unsigned int test_while_loop(unsigned int iterations) {
    volatile unsigned int local_sink = 0;
    unsigned int n = iterations;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        local_sink += n;
        extern_array[n % 1000] = n;  /* External side effect */
    }
    
    return local_sink;
}

/* Function C: Nested loops with pattern in inner loop */
int test_nested_loops(int outer_iter, int inner_iter) {
    volatile int total = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        /* Inner loop uses the decrement-and-compare pattern */
        for (int i = inner_iter; i-- > 0;) {
            total += i * o;
            dummy_side_effect(i);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int test_param_loop(int count) {
    volatile int result = 0;
    
    /* Counter comes from parameter, not constant */
    while (count--) {
        result += count;
        extern_array[count % 1000] = result;  /* External side effect */
    }
    
    return result;
}

/* Function E: Loop with volatile bound (prevents compile-time folding) */
int test_volatile_bound(void) {
    volatile int bound = 100;
    volatile int sum = 0;
    int i = bound;
    
    /* Volatile bound prevents constant propagation */
    while (i--) {
        sum += i;
        dummy_side_effect(i);
    }
    
    return sum;
}

/* Function F: Do-while loop with decrement-and-compare */
int test_dowhile_loop(int iterations) {
    volatile int sum = 0;
    int i = iterations;
    
    if (i <= 0) return 0;
    
    do {
        sum += i;
        dummy_side_effect(i);
    } while (--i > 0);  /* Decrement and compare at the end */
    
    return sum;
}

/* Main driver that calls all test functions and computes checksum */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern coverage...\n");
    
    /* Initialize extern array */
    for (int i = 0; i < 1000; i++) {
        extern_array[i] = 0;
    }
    
    /* Run all test functions with different parameters */
    checksum += test_basic_for_loop(100);
    printf("  test_basic_for_loop completed\n");
    
    checksum += test_while_loop(200);
    printf("  test_while_loop completed\n");
    
    checksum += test_nested_loops(10, 50);
    printf("  test_nested_loops completed\n");
    
    checksum += test_param_loop(150);
    printf("  test_param_loop completed\n");
    
    checksum += test_volatile_bound();
    printf("  test_volatile_bound completed\n");
    
    checksum += test_dowhile_loop(75);
    printf("  test_dowhile_loop completed\n");
    
    /* Add global sink to checksum to ensure all side effects are counted */
    checksum += global_sink;
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum is non-zero, loops executed successfully.\n");
    
    return 0;
}

/* Define the external array */
int extern_array[10000] = {0};
