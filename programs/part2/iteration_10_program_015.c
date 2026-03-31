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
#include <stdlib.h>

/* Global volatile variable to prevent loop elimination */
volatile int global_sink = 0;

/* External array to create side effects */
extern int extern_array[10000];

/* Dummy function with noinline to prevent inlining */
void __attribute__((noinline)) dummy_side_effect(int value) {
    global_sink += value;
}

/* Function A: Basic for loop with int counter using post-decrement pattern */
int test_basic_for_loop(void) {
    int sum = 0;
    volatile int local_sink = 0;
    
    /* Classic decrement-and-compare pattern: i-- > 0 */
    for (int i = 1000; i-- > 0;) {
        sum += i;
        local_sink = i;  /* Volatile store to prevent elimination */
        dummy_side_effect(i);
    }
    
    return sum + local_sink;
}

/* Function B: While loop with unsigned int counter */
int test_while_loop_unsigned(void) {
    unsigned int n = 500;
    int sum = 0;
    
    /* while(n--) pattern - should generate decrement-and-compare */
    while (n--) {
        sum += n;
        extern_array[n % 10000] = n;  /* External side effect */
        global_sink = n;              /* Global volatile side effect */
    }
    
    return sum;
}

/* Function C: Nested loops where inner loop uses the pattern */
int test_nested_loops(void) {
    int outer_sum = 0;
    volatile int inner_sink = 0;
    
    for (int outer = 10; outer > 0; outer--) {
        int inner_counter = 100;
        
        /* Inner loop with decrement-and-compare pattern */
        while (inner_counter--) {
            outer_sum += inner_counter * outer;
            inner_sink = inner_counter;
            dummy_side_effect(inner_counter);
        }
    }
    
    return outer_sum + inner_sink;
}

/* Function D: Loop with counter as function parameter */
int __attribute__((noinline)) test_param_loop(int count) {
    int sum = 0;
    
    /* Counter from parameter prevents constant propagation */
    for (int i = count; i-- > 0;) {
        sum += i;
        extern_array[i % 10000] = i;  /* External side effect */
        global_sink = i;              /* Prevent dead code elimination */
    }
    
    return sum;
}

/* Function E: Loop with volatile bound to prevent compile-time folding */
int test_volatile_bound_loop(void) {
    volatile int bound = 750;
    int sum = 0;
    int counter;
    
    /* Read volatile once to get bound */
    counter = bound;
    
    /* Decrement-and-compare loop */
    while (counter--) {
        sum += counter;
        dummy_side_effect(counter);
    }
    
    return sum;
}

/* Function F: Do-while loop that should also match the pattern */
int test_do_while_loop(void) {
    int iterations = 300;
    int sum = 0;
    
    if (iterations <= 0) return 0;
    
    /* Do-while with decrement-and-compare */
    do {
        sum += iterations;
        global_sink = iterations;
        iterations--;
    } while (iterations > 0);
    
    return sum;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern coverage...\n");
    
    /* Run all test functions and accumulate checksum */
    checksum += test_basic_for_loop();
    printf("Test A complete\n");
    
    checksum += test_while_loop_unsigned();
    printf("Test B complete\n");
    
    checksum += test_nested_loops();
    printf("Test C complete\n");
    
    checksum += test_param_loop(250);
    printf("Test D complete\n");
    
    checksum += test_volatile_bound_loop();
    printf("Test E complete\n");
    
    checksum += test_do_while_loop();
    printf("Test F complete\n");
    
    /* Final verification */
    printf("Final checksum: %d\n", checksum);
    printf("Global sink value: %d\n", global_sink);
    
    /* Simple validation */
    if (checksum != 0) {
        printf("All loops executed successfully.\n");
        return 0;
    } else {
        printf("Warning: Checksum is zero - loops may have been optimized away.\n");
        return 1;
    }
}

/* Define the external array */
int extern_array[10000] = {0};
