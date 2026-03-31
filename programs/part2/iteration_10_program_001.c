/* test_loop_doloop.c
 * This program generates loops with decrement-and-compare-tail patterns
 * to trigger coverage in GCC's loop-doloop.cc pass lines 136-150.
 */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Volatile sink to ensure side effects */
static volatile int volatile_sink = 0;

/* Non-inline dummy function to prevent optimization */
__attribute__((noinline)) 
static void dummy_operation(int value) {
    volatile_sink += value;
}

/* Function A: Basic for loop with int counter */
__attribute__((optimize("O2")))
int test_basic_for_loop(int iterations) {
    int sum = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        /* Simple body with side effect */
        sum += i;
        dummy_operation(i);
    }
    
    return sum;
}

/* Function B: While loop with unsigned counter */
__attribute__((optimize("O2")))
int test_while_loop(unsigned int count) {
    int sum = 0;
    unsigned int n = count;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        sum += (int)n;
        extern_array[n % 1000] = (int)n;  /* External side effect */
    }
    
    return sum;
}

/* Function C: Nested loops with inner decrement pattern */
__attribute__((optimize("O2")))
int test_nested_loops(int outer_iter, int inner_iter) {
    int total = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        int inner = inner_iter;
        
        /* Inner loop with decrement-and-compare pattern */
        while (inner--) {
            total += o * inner;
            volatile_sink += o + inner;
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
__attribute__((optimize("O2")))
int test_param_counter(int count) {
    int result = 0;
    int i = count;
    
    /* Hide loop bound behind volatile read to prevent optimization */
    volatile int bound = count;
    i = bound;
    
    /* Decrement-and-compare pattern with parameter */
    while (i--) {
        result ^= i;  /* Non-linear operation to prevent simplification */
        dummy_operation(result);
    }
    
    return result;
}

/* Function E: Multiple decrement patterns in same function */
__attribute__((optimize("O2")))
int test_multiple_patterns(int a, int b) {
    int sum = 0;
    
    /* First pattern */
    for (int i = a; i-- > 0;) {
        sum += i * 2;
    }
    
    /* Second pattern with different counter */
    unsigned int j = (unsigned int)b;
    while (j--) {
        sum -= (int)j;
        extern_array[j % 500] = sum;
    }
    
    return sum;
}

/* Function F: Do-while with pre-decrement (should also match pattern) */
__attribute__((optimize("O2")))
int test_dowhile_loop(int count) {
    int i = count;
    int acc = 0;
    
    if (i <= 0) return 0;
    
    do {
        acc += i * i;
        volatile_sink = acc;
    } while (--i > 0);  /* Pre-decrement with compare */
    
    return acc;
}

/* Global variable to prevent constant propagation */
volatile int global_counter = 100;

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Initialize external array */
    for (int i = 0; i < 10000; i++) {
        extern_array[i] = 0;
    }
    
    printf("Testing loop-doloop pattern coverage...\n");
    
    /* Test with various loop sizes to trigger different optimizations */
    checksum += test_basic_for_loop(1000);
    printf("Basic for loop completed\n");
    
    checksum += test_while_loop(500);
    printf("While loop completed\n");
    
    checksum += test_nested_loops(10, 100);
    printf("Nested loops completed\n");
    
    checksum += test_param_counter(300);
    printf("Parameter counter loop completed\n");
    
    checksum += test_multiple_patterns(200, 150);
    printf("Multiple patterns completed\n");
    
    checksum += test_dowhile_loop(250);
    printf("Do-while loop completed\n");
    
    /* Additional test with volatile global */
    int local_counter = global_counter;
    checksum += test_basic_for_loop(local_counter);
    printf("Volatile global test completed\n");
    
    printf("Final checksum: %d\n", checksum);
    printf("Volatile sink value: %d\n", volatile_sink);
    
    /* Verify array was modified */
    int array_sum = 0;
    for (int i = 0; i < 1000; i++) {
        array_sum += extern_array[i];
    }
    printf("Array checksum: %d\n", array_sum);
    
    return 0;
}

/* Define the external array */
int extern_array[10000] = {0};
