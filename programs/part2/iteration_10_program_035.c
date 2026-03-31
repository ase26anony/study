/* test_loop_doloop.c
 * 
 * This program is designed to trigger coverage of the decrement-and-compare
 * pattern matching logic in GCC's loop-doloop pass (loop-doloop.cc lines 136-150).
 * The loops are structured to produce the RTL pattern:
 *   (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * 
 * Compilation suggestions:
 *   gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 test_loop_doloop.c -o test
 *   gcc -O3 -fno-peel-loops -fno-predictive-commoning test_loop_doloop.c -o test_aggressive
 */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Dummy function with side effects */
static void __attribute__((noinline)) dummy_side_effect(int value) {
    /* Use volatile to ensure the call isn't optimized away */
    volatile static int sink = 0;
    sink += value;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_int(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Classic decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sink += i;           /* Volatile side effect */
        sum += i & 0xFF;     /* Simple computation */
    }
    
    return sum;
}

/* Function B: While loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int n) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        sink += n;
        sum += (n * 3) & 0xFF;
        dummy_side_effect(n); /* External side effect */
    }
    
    return sum;
}

/* Function C: Nested loops with inner loop using the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare pattern */
        int i = inner;
        while (i--) {
            sink += o * i;
            total += (o + i) & 0xFF;
            extern_array[o * 100 + i] = o + i; /* External array store */
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    volatile int sink = 0;
    int result = 0;
    
    /* Counter from parameter ensures runtime value */
    for (int i = count; i-- > 0;) {
        sink += i;
        result ^= i;  /* Non-linear computation */
        dummy_side_effect(result);
    }
    
    return result;
}

/* Function E: Loop with volatile bound (prevents compile-time folding) */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 1000;
    volatile int sink = 0;
    int checksum = 0;
    
    int i = bound;
    while (i--) {
        sink += i;
        checksum += i % 256;
        extern_array[i % 1000] = i; /* External side effect */
    }
    
    return checksum;
}

/* Main driver that executes all test functions */
int main(void) {
    int final_checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Execute each test with different parameters */
    final_checksum ^= test_for_loop_int(1000);
    printf("  test_for_loop_int completed\n");
    
    final_checksum ^= test_while_loop_unsigned(500);
    printf("  test_while_loop_unsigned completed\n");
    
    final_checksum ^= test_nested_loops(10, 100);
    printf("  test_nested_loops completed\n");
    
    final_checksum ^= test_param_loop(750);
    printf("  test_param_loop completed\n");
    
    final_checksum ^= test_volatile_bound();
    printf("  test_volatile_bound completed\n");
    
    /* Additional variant: do-while with decrement-and-compare */
    {
        volatile int sink = 0;
        int i = 300;
        int sum = 0;
        
        if (i > 0) {
            do {
                sink += i;
                sum += i & 0xFF;
            } while (i-- > 0);
        }
        final_checksum ^= sum;
        printf("  do_while variant completed\n");
    }
    
    printf("Final checksum: %d\n", final_checksum);
    printf("All loops executed successfully.\n");
    
    return 0;
}

/* Define the external array to prevent linker errors */
int extern_array[10000] = {0};
