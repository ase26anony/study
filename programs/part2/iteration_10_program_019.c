/* test_loop_doloop.c
 * 
 * This program is designed to exercise GCC's loop-doloop optimization pass
 * by generating loops with a specific decrement-and-compare tail pattern:
 *   (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * 
 * The loops use various counter types and nesting to ensure the pattern
 * is preserved until the loop-doloop pass analyzes them.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent loop elimination */
volatile int global_sink = 0;

/* External array to create side effects */
extern int extern_array[10000];

/* Non-inline dummy function to prevent optimization */
static void __attribute__((noinline)) dummy_side_effect(int value) {
    global_sink += value;
}

/* Function A: Basic for loop with int counter, decrement-and-compare pattern */
int __attribute__((optimize("O2"))) test_for_loop_int(int limit) {
    int sum = 0;
    /* Pattern: for (int i = limit; i-- > 0;) */
    for (int i = limit; i-- > 0;) {
        /* Volatile side effect to prevent dead code elimination */
        global_sink += i;
        sum += i;
        /* Store to external array */
        if (i < 10000) extern_array[i] = i;
    }
    return sum;
}

/* Function B: While loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    /* Pattern: while (n--) */
    while (n--) {
        dummy_side_effect(n);
        sum += (int)n;
        if (n < 10000) extern_array[n] = (int)n;
    }
    return sum;
}

/* Function C: Nested loops where inner loop uses the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer_limit, int inner_limit) {
    int total = 0;
    for (int o = 0; o < outer_limit; o++) {
        /* Inner loop with decrement-and-compare pattern */
        int i = inner_limit;
        while (i-- > 0) {
            global_sink += o * i;
            total += o * i;
            if (i < 10000) extern_array[i] = o;
        }
    }
    return total;
}

/* Function D: Loop with counter as function parameter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    int sum = 0;
    /* Hide loop bound behind volatile read to prevent early folding */
    volatile int vol_count = count;
    int j = vol_count;
    
    /* Pattern: while (j--) with parameter counter */
    while (j--) {
        dummy_side_effect(j);
        sum += j;
        if (j < 10000) extern_array[j] = j;
    }
    return sum;
}

/* Function E: Loop with counter stored in global volatile (forces memory read) */
int __attribute__((optimize("O2"))) test_volatile_counter(void) {
    int sum = 0;
    /* Read initial value from volatile global */
    volatile int counter = 100;
    int k = counter;
    
    /* Decrement-and-compare pattern */
    for (; k-- > 0;) {
        global_sink += k;
        sum += k;
        if (k < 10000) extern_array[k] = k;
    }
    return sum;
}

/* Main driver that calls all test functions and computes checksum */
int main(void) {
    int checksum = 0;
    
    /* Initialize external array (simulated) */
    for (int i = 0; i < 10000; i++) {
        extern_array[i] = 0;
    }
    
    printf("Testing loop-doloop pattern coverage...\n");
    
    /* Test A: Basic int loop */
    checksum += test_for_loop_int(1000);
    printf("Test A completed, checksum = %d\n", checksum);
    
    /* Test B: Unsigned while loop */
    checksum += test_while_loop_unsigned(500);
    printf("Test B completed, checksum = %d\n", checksum);
    
    /* Test C: Nested loops */
    checksum += test_nested_loops(10, 100);
    printf("Test C completed, checksum = %d\n", checksum);
    
    /* Test D: Parameter counter */
    checksum += test_param_counter(300);
    printf("Test D completed, checksum = %d\n", checksum);
    
    /* Test E: Volatile counter */
    checksum += test_volatile_counter();
    printf("Test E completed, checksum = %d\n", checksum);
    
    /* Final verification */
    printf("Final checksum: %d\n", checksum);
    printf("Global sink value: %d\n", global_sink);
    
    /* Simple validation */
    if (checksum != 884170) {
        printf("WARNING: Unexpected checksum value!\n");
        return 1;
    }
    
    return 0;
}

/* Define the external array */
int extern_array[10000];
