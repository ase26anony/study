/* loop-doloop-coverage.c
 * Test program to cover decrement-and-compare tail pattern matching
 * in GCC's loop-doloop optimization pass.
 */

#include <stdio.h>
#include <stdint.h>

/* External array to prevent dead code elimination */
volatile int extern_array[10000];
volatile int extern_index = 0;

/* Dummy function with side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    extern_array[extern_index++ % 10000] = value;
}

/* Prevent constant propagation of loop bounds */
volatile int volatile_bound = 1000;

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_decrement(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sink += i;           /* Volatile side effect */
        sum += i & 0xFF;     /* Simple computation */
        dummy_side_effect(i); /* External side effect */
    }
    
    return sum;
}

/* Function B: while loop with unsigned counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int n) {
    volatile unsigned int sink = 0;
    unsigned int sum = 0;
    
    /* Another decrement-and-compare pattern: n-- */
    while (n--) {
        sink += n;
        sum += n % 256;
        extern_array[n % 100] = n; /* Array store side effect */
    }
    
    return sum;
}

/* Function C: Nested loops with inner decrement pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare tail */
        for (int i = inner; i-- > 0;) {
            sink += o * i;
            total += (o + i) & 0xFF;
            dummy_side_effect(i);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant folding) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    volatile int sink = 0;
    int result = 0;
    
    /* Use parameter directly in decrement pattern */
    while (count--) {
        sink += count;
        result ^= count;  /* Non-linear to prevent optimization */
        extern_array[count % 1000] = result;
    }
    
    return result;
}

/* Function E: Loop with volatile bound (prevents compile-time evaluation) */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = volatile_bound;
    volatile int sink = 0;
    int checksum = 0;
    
    /* Bound from volatile variable ensures runtime evaluation */
    for (int i = bound; i-- > 0;) {
        sink += i;
        checksum = (checksum * 31 + i) & 0xFFFF;
        dummy_side_effect(checksum);
    }
    
    return checksum;
}

/* Function F: Do-while converted from for loop (explicit tail check) */
int __attribute__((optimize("O2"))) test_do_while_pattern(int n) {
    volatile int sink = 0;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    /* Manual do-while with decrement-and-compare */
    do {
        sink += n;
        sum += n % 100;
        extern_array[n % 500] = n;
    } while (n-- > 1);  /* Note: careful with boundary */
    
    return sum;
}

/* Main driver that exercises all test functions */
int main(void) {
    int total_checksum = 0;
    
    printf("Starting loop-doloop pattern coverage tests...\n");
    
    /* Test A: Basic for loop */
    int result_a = test_for_loop_decrement(1000);
    total_checksum = (total_checksum * 17 + result_a) & 0xFFFFFF;
    printf("Test A (for loop): %d\n", result_a);
    
    /* Test B: While loop */
    unsigned int result_b = test_while_loop_decrement(500);
    total_checksum = (total_checksum * 19 + result_b) & 0xFFFFFF;
    printf("Test B (while loop): %u\n", result_b);
    
    /* Test C: Nested loops */
    int result_c = test_nested_loops(10, 100);
    total_checksum = (total_checksum * 23 + result_c) & 0xFFFFFF;
    printf("Test C (nested loops): %d\n", result_c);
    
    /* Test D: Parameter counter */
    int result_d = test_param_counter(750);
    total_checksum = (total_checksum * 29 + result_d) & 0xFFFFFF;
    printf("Test D (parameter counter): %d\n", result_d);
    
    /* Test E: Volatile bound */
    int result_e = test_volatile_bound();
    total_checksum = (total_checksum * 31 + result_e) & 0xFFFFFF;
    printf("Test E (volatile bound): %d\n", result_e);
    
    /* Test F: Do-while pattern */
    int result_f = test_do_while_pattern(300);
    total_checksum = (total_checksum * 37 + result_f) & 0xFFFFFF;
    printf("Test F (do-while): %d\n", result_f);
    
    printf("Total checksum: %d\n", total_checksum);
    printf("Tests completed.\n");
    
    return 0;
}
