/* test_loop_doloop.c
 * 
 * This program generates loops that should produce the RTL pattern:
 * (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * which is matched in GCC's loop-doloop.cc lines 136-150.
 */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Volatile sink to ensure loop bodies aren't optimized away */
static volatile int volatile_sink = 0;

/* Non-inline function to create side effects */
__attribute__((noinline)) 
static void dummy_operation(int value) {
    volatile_sink += value;
}

/* Function A: Basic for loop with int counter using decrement-and-compare */
__attribute__((optimize("O2")))
int test_basic_for_loop(int iterations) {
    int sum = 0;
    /* Pattern: for (int i = N; i-- > 0;) */
    for (int i = iterations; i-- > 0;) {
        /* Simple side effect that can't be optimized away */
        sum += i;
        dummy_operation(i);
    }
    return sum;
}

/* Function B: While loop with unsigned int counter */
__attribute__((optimize("O2")))
int test_while_loop(unsigned int count) {
    int result = 0;
    unsigned int n = count;
    
    /* Pattern: while (n--) */
    while (n--) {
        result += (int)n;
        extern_array[n % 1000] = (int)n;  /* External side effect */
        volatile_sink ^= result;          /* Volatile access */
    }
    return result;
}

/* Function C: Nested loops with inner loop using the pattern */
__attribute__((optimize("O2")))
int test_nested_loops(int outer_iter, int inner_iter) {
    int total = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        int inner_counter = inner_iter;
        
        /* Inner loop with decrement-and-compare pattern */
        while (inner_counter--) {
            total += inner_counter * o;
            /* Use volatile to prevent optimization */
            *(volatile int*)&volatile_sink = total;
        }
    }
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
__attribute__((optimize("O2")))
int test_param_loop(int count) {
    int checksum = 0;
    int local_count = count;  /* Copy to local to ensure decrement happens */
    
    /* Explicit decrement-and-compare pattern */
    for (; local_count-- > 0; ) {
        checksum = checksum * 31 + local_count;
        dummy_operation(checksum);
    }
    return checksum;
}

/* Function E: Loop with volatile bound to prevent compile-time folding */
__attribute__((optimize("O2")))
int test_volatile_bound(void) {
    volatile int volatile_bound = 100;
    int bound = volatile_bound;  /* Read from volatile */
    int accumulator = 0;
    
    /* Classic decrement-and-compare loop */
    while (bound--) {
        accumulator += bound * 2;
        extern_array[bound % 100] = accumulator;
    }
    return accumulator;
}

/* Function F: Do-while style that should also match the pattern */
__attribute__((optimize("O2")))
int test_do_while_pattern(int limit) {
    int counter = limit;
    int value = 0;
    
    if (counter <= 0) return 0;
    
    do {
        value += counter * 3;
        volatile_sink = value;  /* Volatile write */
    } while (counter-- > 1);  /* Note: careful with do-while decrement */
    
    return value;
}

/* Main driver that runs all tests and computes final checksum */
int main(void) {
    int final_checksum = 0;
    
    printf("Testing loop patterns for GCC loop-doloop pass coverage\n");
    
    /* Test A: Basic for loop */
    int result_a = test_basic_for_loop(1000);
    final_checksum += result_a;
    printf("Test A (basic for): %d\n", result_a);
    
    /* Test B: While loop with unsigned */
    int result_b = test_while_loop(500);
    final_checksum += result_b;
    printf("Test B (while unsigned): %d\n", result_b);
    
    /* Test C: Nested loops */
    int result_c = test_nested_loops(10, 100);
    final_checksum += result_c;
    printf("Test C (nested): %d\n", result_c);
    
    /* Test D: Parameter loop */
    int result_d = test_param_loop(750);
    final_checksum += result_d;
    printf("Test D (param): %d\n", result_d);
    
    /* Test E: Volatile bound */
    int result_e = test_volatile_bound();
    final_checksum += result_e;
    printf("Test E (volatile bound): %d\n", result_e);
    
    /* Test F: Do-while pattern */
    int result_f = test_do_while_pattern(300);
    final_checksum += result_f;
    printf("Test F (do-while): %d\n", result_f);
    
    printf("Final checksum: %d\n", final_checksum);
    
    /* Ensure all volatile operations aren't optimized away */
    printf("Volatile sink value: %d\n", volatile_sink);
    
    return 0;
}

/* Define the external array to avoid linker errors */
int extern_array[10000] = {0};
