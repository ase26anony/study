/* test_loop_doloop.c
 * 
 * This program is designed to trigger coverage of the decrement-and-compare
 * pattern matching logic in GCC's loop-doloop pass (loop-doloop.cc lines 136-150).
 * Each test function contains a loop that should compile to the RTL pattern:
 *   (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * The loops use volatile variables and external side effects to prevent
 * premature optimization.
 */

#include <stdio.h>
#include <stdint.h>

/* External array to create side effects */
extern volatile int extern_array[1024];

/* Dummy function with side effects to prevent loop removal */
static void __attribute__((noinline)) dummy_side_effect(int value) {
    extern_array[value & 1023] = value;
}

/* Function A: Basic for loop with int counter, decrement-and-compare pattern */
int __attribute__((optimize("O2"))) test_for_loop_int(int limit) {
    volatile int sink = 0;
    int sum = 0;
    
    /* for (int i = limit; i-- > 0;) produces the desired pattern */
    for (int i = limit; i-- > 0;) {
        sink += i;           /* Volatile access prevents dead code elimination */
        sum += i & 0xFF;     /* Simple computation for verification */
        dummy_side_effect(i); /* External side effect */
    }
    
    return sum & 0xFFFF;
}

/* Function B: while loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int limit) {
    volatile int sink = 0;
    unsigned int n = limit;
    int sum = 0;
    
    /* while (n--) produces the desired pattern */
    while (n--) {
        sink += n;
        sum += (n * 3) & 0xFF;
        dummy_side_effect(n);
    }
    
    return sum & 0xFFFF;
}

/* Function C: Nested loops where inner loop uses the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer_limit, int inner_limit) {
    volatile int sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer_limit; o++) {
        /* Inner loop with decrement-and-compare pattern */
        for (int i = inner_limit; i-- > 0;) {
            sink += o * i;
            total += (o + i) & 0xFF;
            dummy_side_effect(i);
        }
    }
    
    return total & 0xFFFF;
}

/* Function D: Loop with counter as function parameter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    volatile int sink = 0;
    int result = 0;
    
    /* count parameter prevents compile-time folding */
    while (count--) {
        sink += count;
        result ^= count;  /* Non-linear operation to prevent simplification */
        dummy_side_effect(count);
    }
    
    return result & 0xFFFF;
}

/* Function E: Do-while loop that also matches the pattern */
int __attribute__((optimize("O2"))) test_dowhile_loop(int iterations) {
    volatile int sink = 0;
    int i = iterations;
    int checksum = 0;
    
    if (i <= 0) return 0;
    
    do {
        sink += i;
        checksum = (checksum * 31 + i) & 0xFFFF;
        dummy_side_effect(i);
    } while (i-- > 1);  /* Note: careful with do-while to match pattern */
    
    return checksum;
}

/* Global volatile to hide loop bounds from constant propagation */
volatile int hidden_bound = 100;

/* Function F: Loop with volatile bound */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int sink = 0;
    int bound = hidden_bound;  /* Volatile read prevents constant propagation */
    int sum = 0;
    
    for (int i = bound; i-- > 0;) {
        sink += i;
        sum += i % 17;
        dummy_side_effect(i);
    }
    
    return sum & 0xFFFF;
}

/* Main function that drives all tests and verifies correctness */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Run each test with different parameters */
    checksum ^= test_for_loop_int(1000);
    checksum ^= test_while_loop_unsigned(500);
    checksum ^= test_nested_loops(10, 50);
    checksum ^= test_param_counter(300);
    checksum ^= test_dowhile_loop(200);
    checksum ^= test_volatile_bound();
    
    /* Add some variation with different bounds */
    checksum ^= test_for_loop_int(1);     /* Edge case: 1 iteration */
    checksum ^= test_while_loop_unsigned(0); /* Edge case: 0 iterations */
    
    printf("Final checksum: 0x%04X\n", checksum & 0xFFFF);
    
    /* Return non-zero if any test failed (simplified check) */
    return (checksum == 0) ? 1 : 0;
}

/* Definition of the external array */
volatile int extern_array[1024] = {0};
