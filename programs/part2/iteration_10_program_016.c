/* loop-doloop-coverage.c
 * Test program to cover decrement-and-compare tail pattern matching
 * in GCC's loop-doloop optimization pass.
 */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Volatile sink to prevent loop removal */
volatile int volatile_sink = 0;

/* Non-inline dummy function with side effect */
void __attribute__((noinline)) dummy_side_effect(int value) {
    volatile_sink += value;
}

/* Prevent constant propagation of loop bounds */
volatile int volatile_bound = 1000;

/* Function A: Basic for loop with int counter using post-decrement pattern */
int __attribute__((optimize("O2"))) test_for_loop_postdecrement(int iterations) {
    int sum = 0;
    /* Classic post-decrement compare-to-zero pattern */
    for (int i = iterations; i-- > 0;) {
        /* Simple side effect that can't be optimized away */
        sum += i;
        extern_array[i & 1023] = i;  /* Mask to avoid out-of-bounds */
        dummy_side_effect(i);
    }
    return sum;
}

/* Function B: while loop with unsigned int counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int n) {
    unsigned int checksum = 0;
    /* while(n--) pattern - should generate decrement-and-compare */
    while (n--) {
        checksum ^= n;  /* Non-trivial computation */
        volatile_sink = n;  /* Volatile store */
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
            extern_array[(o * inner + i) & 1023] = total;
        }
    }
    return total;
}

/* Function D: Loop with parameter counter (prevents constant folding) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    int result = 0;
    /* Use parameter directly in loop condition */
    while (count--) {
        result += count * 2;
        dummy_side_effect(count);
    }
    return result;
}

/* Function E: Loop with volatile bound to prevent compile-time evaluation */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    int local_sink = 0;
    int bound = volatile_bound;  /* Read from volatile to prevent const prop */
    
    /* This should generate the target pattern */
    for (int i = bound; i-- > 0;) {
        local_sink += i * 3;
        extern_array[i & 1023] = local_sink;
    }
    return local_sink;
}

/* Function F: Do-while loop that already has condition at end */
int __attribute__((optimize("O2"))) test_dowhile_pattern(int n) {
    int acc = 0;
    if (n <= 0) return 0;
    
    /* do-while with post-decrement */
    do {
        acc += n;
        dummy_side_effect(n);
    } while (n-- > 1);  /* Note: careful with boundary */
    
    return acc;
}

/* Function G: Loop with different integer type (short) */
short __attribute__((optimize("O2"))) test_short_counter(short limit) {
    short sum = 0;
    for (short s = limit; s-- > 0;) {
        sum += s;
        volatile_sink = s;  /* Ensure side effect */
    }
    return sum;
}

/* Main driver that runs all tests and computes final checksum */
int main(void) {
    int final_checksum = 0;
    
    printf("Running loop-doloop pattern tests...\n");
    
    /* Test A: Basic for loop */
    int result_a = test_for_loop_postdecrement(500);
    final_checksum ^= result_a;
    printf("Test A (for loop): %d\n", result_a);
    
    /* Test B: while loop with unsigned */
    unsigned int result_b = test_while_loop_unsigned(300);
    final_checksum ^= (int)result_b;
    printf("Test B (while unsigned): %u\n", result_b);
    
    /* Test C: Nested loops */
    int result_c = test_nested_loops(10, 50);
    final_checksum ^= result_c;
    printf("Test C (nested): %d\n", result_c);
    
    /* Test D: Parameter counter */
    int result_d = test_param_counter(400);
    final_checksum ^= result_d;
    printf("Test D (param counter): %d\n", result_d);
    
    /* Test E: Volatile bound */
    int result_e = test_volatile_bound();
    final_checksum ^= result_e;
    printf("Test E (volatile bound): %d\n", result_e);
    
    /* Test F: Do-while */
    int result_f = test_dowhile_pattern(200);
    final_checksum ^= result_f;
    printf("Test F (do-while): %d\n", result_f);
    
    /* Test G: Short counter */
    short result_g = test_short_counter(100);
    final_checksum ^= result_g;
    printf("Test G (short counter): %d\n", result_g);
    
    printf("\nFinal checksum: %d\n", final_checksum);
    printf("Volatile sink (side effect verification): %d\n", volatile_sink);
    
    return 0;
}

/* Define the external array */
int extern_array[10000] = {0};
