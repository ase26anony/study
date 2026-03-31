/* test_loop_doloop.c
 * 
 * This program is designed to exercise the loop-doloop pass in GCC,
 * specifically targeting the RTL pattern matching for decrement-and-compare
 * loop tails: (set (reg:CC) (compare (plus (reg) -1) (const0)))
 *
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 test_loop_doloop.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent dead code elimination */
volatile int global_sink = 0;

/* External array to create side effects */
extern int extern_array[10000];

/* Non-inline dummy function to create side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    global_sink += value;
}

/* Function A: Basic for loop with int counter using decrement-and-compare pattern */
int __attribute__((optimize("O2"))) test_for_loop_decrement(int iterations) {
    volatile int local_sink = 0;
    
    /* Classic decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        local_sink += i;
        extern_array[i] = i * 2;
    }
    
    return local_sink;
}

/* Function B: While loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int count) {
    int sum = 0;
    unsigned int n = count;
    
    /* while(n--) pattern - decrements then compares to zero */
    while (n--) {
        sum += (int)n;
        dummy_side_effect(n);
    }
    
    return sum;
}

/* Function C: Nested loops where inner loop uses the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer_iter, int inner_iter) {
    int total = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        volatile int inner_sink = 0;
        int i = inner_iter;
        
        /* Inner loop with decrement-and-compare */
        while (i--) {
            inner_sink += i + o;
            extern_array[i] = i + o;
        }
        
        total += inner_sink;
    }
    
    return total;
}

/* Function D: Loop with counter as function parameter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    volatile int result = 0;
    int c = count;
    
    /* Decrement-and-compare with parameterized count */
    do {
        result += c;
        dummy_side_effect(c);
    } while (c-- > 0);
    
    return result;
}

/* Function E: Additional test with volatile loop bound */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 100;
    int sum = 0;
    int i = bound;
    
    /* Prevents the compiler from knowing exact iteration count */
    while (i--) {
        sum += i;
        extern_array[i % 1000] = sum;
    }
    
    return sum;
}

/* Function F: Loop with different integer type (short) */
int __attribute__((optimize("O2"))) test_short_counter(void) {
    short counter = 50;
    int total = 0;
    
    /* short counter decrement-and-compare */
    while (counter--) {
        total += counter;
        global_sink += counter;
    }
    
    return total;
}

/* Main driver that calls all test functions and computes checksum */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern coverage...\n");
    
    /* Initialize extern array (simulated) */
    for (int i = 0; i < 10000; i++) {
        extern_array[i] = 0;
    }
    
    /* Run all test functions with different parameters */
    checksum += test_for_loop_decrement(100);
    printf("Test A complete\n");
    
    checksum += test_while_loop_decrement(200);
    printf("Test B complete\n");
    
    checksum += test_nested_loops(5, 40);
    printf("Test C complete\n");
    
    checksum += test_param_loop(75);
    printf("Test D complete\n");
    
    checksum += test_volatile_bound();
    printf("Test E complete\n");
    
    checksum += test_short_counter();
    printf("Test F complete\n");
    
    /* Add some noise to prevent optimization */
    checksum += global_sink;
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum != 0, loops executed successfully.\n");
    
    return 0;
}

/* Define the extern array */
int extern_array[10000];
