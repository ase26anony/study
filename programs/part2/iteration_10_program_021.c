/* test_loop_doloop.c
 * 
 * This program is designed to exercise GCC's loop-doloop optimization pass
 * by generating loops with the specific decrement-and-compare pattern:
 *   (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * 
 * The loops use various counter types and nesting structures to ensure
 * the pattern is preserved through compilation and matched by the pass.
 */

#include <stdio.h>
#include <stdint.h>

/* Global volatile variable to prevent loop elimination */
volatile int g_volatile_sink = 0;

/* External array to create side effects */
extern int g_extern_array[10000];

/* Non-inline dummy function to create side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    g_volatile_sink += value;
}

/* Function A: Basic for loop with int counter, decrement-and-compare pattern */
int __attribute__((optimize("O2"))) test_for_loop_int(int iterations) {
    int sum = 0;
    /* Pattern: for (int i = N; i-- > 0;) */
    for (int i = iterations; i-- > 0;) {
        /* Simple side effect to prevent elimination */
        sum += i;
        dummy_side_effect(i);
    }
    return sum;
}

/* Function B: while loop with unsigned int counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int n) {
    unsigned int checksum = 0;
    /* Pattern: while (n--) */
    while (n--) {
        checksum ^= n;  /* Non-trivial computation */
        g_extern_array[n % 1000] = n;  /* External array store */
    }
    return checksum;
}

/* Function C: Nested loops with inner loop using the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    int total = 0;
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare pattern */
        for (int i = inner; i-- > 0;) {
            total += o * i;
            g_volatile_sink = total;  /* Volatile store */
        }
    }
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    int result = 0;
    /* Use volatile intermediate to hide loop bound */
    volatile int vol_count = count;
    
    /* Pattern: while (count--) with parameter counter */
    while (vol_count--) {
        result += vol_count * 2;
        dummy_side_effect(result);
    }
    return result;
}

/* Function E: Loop with counter hidden behind memory read */
int __attribute__((optimize("O2"))) test_memory_bound_loop(void) {
    static volatile int loop_bound = 100;
    int accumulator = 0;
    int counter = loop_bound;  /* Read from volatile */
    
    /* Pattern: for (; counter-- > 0;) */
    for (; counter-- > 0;) {
        accumulator += counter;
        g_extern_array[accumulator % 1000] = accumulator;
    }
    return accumulator;
}

/* Main driver that runs all tests and computes final checksum */
int main(void) {
    int final_checksum = 0;
    
    /* Initialize external array (simulated) */
    for (int i = 0; i < 1000; i++) {
        g_extern_array[i] = 0;
    }
    
    /* Test A: Basic int loop */
    int result_a = test_for_loop_int(1000);
    final_checksum += result_a;
    printf("Test A result: %d\n", result_a);
    
    /* Test B: Unsigned while loop */
    unsigned int result_b = test_while_loop_unsigned(500);
    final_checksum += (int)result_b;
    printf("Test B result: %u\n", result_b);
    
    /* Test C: Nested loops */
    int result_c = test_nested_loops(10, 100);
    final_checksum += result_c;
    printf("Test C result: %d\n", result_c);
    
    /* Test D: Parameter loop */
    int result_d = test_param_loop(300);
    final_checksum += result_d;
    printf("Test D result: %d\n", result_d);
    
    /* Test E: Memory-bound loop */
    int result_e = test_memory_bound_loop();
    final_checksum += result_e;
    printf("Test E result: %d\n", result_e);
    
    printf("Final checksum: %d\n", final_checksum);
    
    /* Verify by recomputing expected values */
    int expected = 0;
    for (int i = 1000; i-- > 0;) expected += i;
    
    unsigned int n = 500;
    while (n--) expected += n;
    
    for (int o = 0; o < 10; o++) {
        for (int i = 100; i-- > 0;) {
            expected += o * i;
        }
    }
    
    int count = 300;
    while (count--) expected += count * 2;
    
    int counter = 100;
    for (; counter-- > 0;) expected += counter;
    
    printf("Expected checksum: %d\n", expected);
    printf("Verification: %s\n", (final_checksum == expected) ? "PASS" : "FAIL");
    
    return (final_checksum == expected) ? 0 : 1;
}

/* Define the external array */
int g_extern_array[10000] = {0};
