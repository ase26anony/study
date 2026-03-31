/* test_loop_doloop.c
 * 
 * This program is designed to trigger coverage of the decrement-and-compare
 * pattern matching logic in GCC's loop-doloop pass (loop-doloop.cc lines 136-150).
 * The loops are structured to produce the RTL pattern:
 *   (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * which corresponds to a counter decrement followed by comparison against zero.
 */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern volatile int extern_array[1024];

/* Dummy function with side effects to prevent loop removal */
static void __attribute__((noinline)) dummy_side_effect(int value) {
    extern_array[value & 1023] = value;
}

/* Volatile sink to ensure loop bodies aren't optimized away */
static volatile int volatile_sink = 0;

/* Global variable to hide loop bounds from constant propagation */
int global_bound = 1000;

/* Function A: Basic for loop with int counter using post-decrement pattern */
int __attribute__((optimize("O2"))) test_for_loop_int(int iterations) {
    int sum = 0;
    /* The canonical pattern: for (int i = N; i-- > 0;) */
    for (int i = iterations; i-- > 0;) {
        /* Simple side effect that can't be optimized away */
        volatile_sink += i;
        sum += i;
    }
    return sum;
}

/* Function B: While loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int n) {
    int sum = 0;
    /* Pattern: while (n--) */
    while (n--) {
        /* Use dummy function to ensure side effects */
        dummy_side_effect(n);
        sum += (int)n;
    }
    return sum;
}

/* Function C: Nested loops where inner loop uses the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    int total = 0;
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare pattern */
        for (int i = inner; i-- > 0;) {
            /* Array access prevents optimization */
            extern_array[(o * inner + i) & 1023] = i;
            total += i;
        }
    }
    return total;
}

/* Function D: Loop with parameter counter to prevent constant propagation */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    int result = 0;
    /* Counter comes from parameter, not a constant */
    while (count--) {
        /* Multiple side effects to prevent transformation */
        volatile_sink ^= count;
        result += count;
        dummy_side_effect(count);
    }
    return result;
}

/* Function E: Loop with volatile bound to prevent compile-time folding */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int volatile_counter = 500;
    int sum = 0;
    int counter = volatile_counter;
    
    /* The volatile read forces runtime evaluation */
    while (counter--) {
        sum += counter;
        extern_array[counter & 1023] = sum;
    }
    return sum;
}

/* Function F: Do-while loop that should also match the pattern */
int __attribute__((optimize("O2"))) test_do_while_loop(int limit) {
    int i = limit;
    int sum = 0;
    
    if (i <= 0) return 0;
    
    do {
        sum += i;
        volatile_sink = i;
    } while (i-- > 1);  /* Note: careful with do-while post-decrement */
    
    return sum;
}

/* Main driver that executes all test functions and computes checksum */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test each function with different parameters */
    checksum += test_for_loop_int(1000);
    printf("test_for_loop_int completed\n");
    
    checksum += test_while_loop_unsigned(500);
    printf("test_while_loop_unsigned completed\n");
    
    checksum += test_nested_loops(10, 100);
    printf("test_nested_loops completed\n");
    
    checksum += test_param_loop(750);
    printf("test_param_loop completed\n");
    
    checksum += test_volatile_bound();
    printf("test_volatile_bound completed\n");
    
    checksum += test_do_while_loop(300);
    printf("test_do_while_loop completed\n");
    
    printf("Final checksum: %d\n", checksum);
    
    /* Expected values for verification:
     * test_for_loop_int(1000): sum = 0+1+...+999 = 499500
     * test_while_loop_unsigned(500): sum = 0+1+...+499 = 124750
     * test_nested_loops(10,100): sum = 10 * (0+1+...+99) = 10 * 4950 = 49500
     * test_param_loop(750): sum = 0+1+...+749 = 280875
     * test_volatile_bound(): sum = 0+1+...+499 = 124750
     * test_do_while_loop(300): sum = 300+299+...+1 = 45150
     * Total: 499500 + 124750 + 49500 + 280875 + 124750 + 45150 = 1,124,525
     */
    
    if (checksum == 1124525) {
        printf("All loops executed correctly\n");
        return 0;
    } else {
        printf("Unexpected checksum: %d (expected: 1124525)\n", checksum);
        return 1;
    }
}

/* Define the external array */
volatile int extern_array[1024] = {0};
