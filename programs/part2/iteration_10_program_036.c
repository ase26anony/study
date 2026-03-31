/* test_loop_doloop.c
 * 
 * This program is designed to exercise the loop-doloop pass in GCC
 * by generating loops with a decrement-and-compare tail pattern:
 *   (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * 
 * The loops use various counter types and nesting to ensure the
 * pattern is preserved through optimization passes.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent loop elimination */
volatile int global_sink = 0;

/* External array to create side effects */
extern int extern_array[10000];

/* Non-inlinable dummy function to create side effects */
void __attribute__((noinline)) dummy_side_effect(int x) {
    global_sink += x;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_int(int iterations) {
    volatile int local_sink = 0;
    int sum = 0;
    
    /* The canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sum += i;
        local_sink = sum;  /* Volatile store prevents dead code elimination */
        dummy_side_effect(i);
    }
    
    return sum;
}

/* Function B: While loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int iterations) {
    volatile int local_sink = 0;
    int sum = 0;
    unsigned int n = iterations;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        sum += (int)n;
        local_sink = sum;
        extern_array[n % 10000] = (int)n;  /* External side effect */
    }
    
    return sum;
}

/* Function C: Nested loops with inner loop using the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer_iter, int inner_iter) {
    volatile int local_sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        /* Inner loop with decrement-and-compare pattern */
        for (int i = inner_iter; i-- > 0;) {
            total += i * o;
            local_sink = total;
            dummy_side_effect(i);
        }
    }
    
    return total;
}

/* Function D: Loop with counter as function parameter */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    volatile int local_sink = 0;
    int result = 0;
    
    /* Counter is parameter - prevents constant propagation */
    while (count--) {
        result += count * 2;
        local_sink = result;
        extern_array[count % 10000] = result;  /* External side effect */
    }
    
    return result;
}

/* Function E: Loop with volatile bound to prevent compile-time folding */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 1000;
    volatile int local_sink = 0;
    int sum = 0;
    int i = bound;
    
    /* Loop with volatile-initialized counter */
    while (i-- > 0) {
        sum += i;
        local_sink = sum;
        dummy_side_effect(i);
    }
    
    return sum;
}

/* Main driver that runs all tests and computes checksum */
int main(void) {
    int checksum = 0;
    
    printf("Running loop-doloop pattern tests...\n");
    
    /* Test A: Basic int for loop */
    int result_a = test_for_loop_int(1000);
    checksum += result_a;
    printf("Test A result: %d\n", result_a);
    
    /* Test B: Unsigned while loop */
    int result_b = test_while_loop_unsigned(500);
    checksum += result_b;
    printf("Test B result: %d\n", result_b);
    
    /* Test C: Nested loops */
    int result_c = test_nested_loops(10, 100);
    checksum += result_c;
    printf("Test C result: %d\n", result_c);
    
    /* Test D: Parameter loop */
    int result_d = test_param_loop(750);
    checksum += result_d;
    printf("Test D result: %d\n", result_d);
    
    /* Test E: Volatile bound loop */
    int result_e = test_volatile_bound();
    checksum += result_e;
    printf("Test E result: %d\n", result_e);
    
    printf("Final checksum: %d\n", checksum);
    printf("Global side effect tracker: %d\n", global_sink);
    
    return 0;
}

/* External array definition */
int extern_array[10000] = {0};
