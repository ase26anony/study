/* test_loop_doloop.c
 * 
 * This program is designed to trigger coverage of the decrement-and-compare
 * pattern matching logic in GCC's loop-doloop.cc pass (lines 136-150).
 * It contains multiple loops that should compile to the RTL pattern:
 *   (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * Each loop uses a volatile side effect to prevent optimization removal.
 */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Dummy function with side effect */
static void __attribute__((noinline)) dummy_side_effect(int value) {
    extern_array[value % 10000] = value;
}

/* Volatile sink to force loop execution */
static volatile int volatile_sink = 0;

/* Function A: Basic for loop with int counter, decrement-and-compare pattern */
int __attribute__((optimize("O2"))) test_for_loop_decrement(int iterations) {
    int sum = 0;
    volatile_sink = 0;
    
    /* for (int i = iterations; i-- > 0;) produces the desired pattern */
    for (int i = iterations; i-- > 0;) {
        /* Volatile side effect prevents loop removal */
        volatile_sink += i;
        sum += i;
    }
    return sum;
}

/* Function B: while loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int iterations) {
    int sum = 0;
    unsigned int n = iterations;
    volatile_sink = 0;
    
    /* while (n--) produces the desired pattern */
    while (n--) {
        volatile_sink += n;
        sum += (int)n;
        dummy_side_effect(n);
    }
    return sum;
}

/* Function C: Nested loops where inner loop uses the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer_iter, int inner_iter) {
    int total = 0;
    volatile_sink = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        /* Inner loop with decrement-and-compare pattern */
        for (int i = inner_iter; i-- > 0;) {
            volatile_sink += i + o;
            total += i;
        }
    }
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    int sum = 0;
    volatile_sink = 0;
    
    /* count is a parameter, not a constant */
    for (int i = count; i-- > 0;) {
        volatile_sink += i;
        sum += i;
        dummy_side_effect(i);
    }
    return sum;
}

/* Function E: Loop with volatile bound (prevents compile-time folding) */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    int sum = 0;
    volatile int volatile_bound = 100;
    volatile_sink = 0;
    
    /* Volatile bound ensures decrement happens at runtime */
    for (int i = volatile_bound; i-- > 0;) {
        volatile_sink += i;
        sum += i;
    }
    return sum;
}

/* Function F: Do-while loop that should also match the pattern */
int __attribute__((optimize("O2"))) test_dowhile_loop(int iterations) {
    int sum = 0;
    int i = iterations;
    volatile_sink = 0;
    
    if (i > 0) {
        do {
            volatile_sink += i;
            sum += i;
            dummy_side_effect(i);
        } while (i-- > 0);
    }
    return sum;
}

/* Main function that drives all tests and verifies correctness */
int main(void) {
    int checksum = 0;
    
    printf("Testing loops for loop-doloop.cc pattern matching...\n");
    
    /* Test each function with different parameters */
    checksum += test_for_loop_decrement(100);
    checksum += test_while_loop_decrement(200);
    checksum += test_nested_loops(10, 15);
    checksum += test_param_loop(150);
    checksum += test_volatile_bound();
    checksum += test_dowhile_loop(50);
    
    /* Verify with expected values */
    int expected = 0;
    /* Sum of 0..99 */ expected += 99 * 100 / 2;
    /* Sum of 0..199 */ expected += 199 * 200 / 2;
    /* 10 * sum of 0..14 */ expected += 10 * (14 * 15 / 2);
    /* Sum of 0..149 */ expected += 149 * 150 / 2;
    /* Sum of 0..99 */ expected += 99 * 100 / 2;
    /* Sum of 1..50 (do-while executes at least once) */ expected += 50 * 51 / 2;
    
    printf("Checksum: %d (expected: %d)\n", checksum, expected);
    
    if (checksum == expected) {
        printf("All loops executed correctly.\n");
        return 0;
    } else {
        printf("Error: Checksum mismatch!\n");
        return 1;
    }
}

/* Define the external array */
int extern_array[10000] = {0};
