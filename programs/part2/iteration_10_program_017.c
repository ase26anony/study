/* test_loop_doloop.c
 * 
 * This program is designed to exercise GCC's loop-doloop optimization pass
 * by generating loops with the specific decrement-and-compare pattern:
 *   (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * 
 * Each test function creates a loop that should compile to this RTL pattern,
 * with variations to explore different code generation paths.
 */

#include <stdio.h>
#include <stdint.h>

/* Global variables to prevent constant propagation and dead code elimination */
volatile int g_volatile_sink = 0;
int g_array[1000];
extern int external_var; /* Declared but not defined to prevent optimization */

/* Dummy function with side effects to prevent loop removal */
static void __attribute__((noinline)) dummy_side_effect(int value) {
    g_volatile_sink += value;
}

/* Function A: Basic for loop with int counter using post-decrement pattern */
int __attribute__((optimize("O2"))) test_for_loop_decrement(int iterations) {
    int sum = 0;
    /* Classic pattern: for (int i = N; i-- > 0;) */
    for (int i = iterations; i-- > 0;) {
        /* Simple side effect to prevent removal */
        sum += i;
        dummy_side_effect(i);
    }
    return sum;
}

/* Function B: While loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int n) {
    int sum = 0;
    /* Pattern: while (n--) */
    while (n--) {
        sum += (int)n;
        g_array[n % 1000] = n; /* Array store for side effect */
    }
    return sum;
}

/* Function C: Nested loops where inner loop uses the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    int total = 0;
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare pattern */
        int i = inner;
        while (i--) {
            total += o * i;
            dummy_side_effect(i);
        }
    }
    return total;
}

/* Function D: Loop with counter as function parameter (prevents constant folding) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    int result = 0;
    /* Use volatile to hide the actual bound from the optimizer */
    volatile int vol_count = count;
    
    /* Loop with parameter-based counter */
    int i = vol_count;
    do {
        result += i;
        g_volatile_sink = i; /* Volatile store ensures side effect */
    } while (i-- > 0);
    
    return result;
}

/* Function E: Additional test with different integer type */
int64_t __attribute__((optimize("O2"))) test_64bit_counter(int32_t count) {
    int64_t sum = 0;
    int64_t i = count;
    
    /* 64-bit decrement-and-compare */
    while (i--) {
        sum += i;
        if ((i & 0xFF) == 0) {
            dummy_side_effect((int)i); /* Occasional side effect */
        }
    }
    return sum;
}

/* Main function that drives all tests and verifies correctness */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern coverage...\n");
    
    /* Test A: Basic for loop */
    int result_a = test_for_loop_decrement(1000);
    checksum += result_a;
    printf("Test A result: %d\n", result_a);
    
    /* Test B: While loop with unsigned */
    int result_b = test_while_loop_decrement(500);
    checksum += result_b;
    printf("Test B result: %d\n", result_b);
    
    /* Test C: Nested loops */
    int result_c = test_nested_loops(10, 100);
    checksum += result_c;
    printf("Test C result: %d\n", result_c);
    
    /* Test D: Parameter counter */
    int result_d = test_param_counter(300);
    checksum += result_d;
    printf("Test D result: %d\n", result_d);
    
    /* Test E: 64-bit counter */
    int64_t result_e = test_64bit_counter(200);
    checksum += (int)result_e;
    printf("Test E result: %ld\n", (long)result_e);
    
    /* Final checksum */
    printf("Total checksum: %d\n", checksum);
    
    /* Verify with expected values (computed manually for these inputs) */
    int expected_a = 499500; /* sum(0..999) */
    int expected_b = 124750; /* sum(0..499) */
    int expected_c = 49500;  /* 10 * sum(0..99) */
    int expected_d = 45150;  /* sum(0..300) */
    int64_t expected_e = 19900; /* sum(0..199) */
    
    int expected_checksum = expected_a + expected_b + expected_c + expected_d + (int)expected_e;
    
    if (checksum == expected_checksum) {
        printf("All loops executed correctly!\n");
        return 0;
    } else {
        printf("Mismatch! Expected %d, got %d\n", expected_checksum, checksum);
        return 1;
    }
}
