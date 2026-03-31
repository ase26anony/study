/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdint.h>

/* External array to prevent dead code elimination */
volatile int extern_array[10000];
volatile int extern_sink = 0;

/* Non-inline dummy function to create side effects */
__attribute__((noinline)) 
static void dummy_side_effect(int value) {
    extern_sink += value;
}

/* Prevent constant propagation of loop bounds */
static volatile int volatile_bound = 0;

/* Function A: Basic for loop with int counter */
__attribute__((optimize("O2")))
int test_for_loop_decrement(int iterations) {
    int sum = 0;
    volatile int local_sink = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        /* Simple side effect to prevent loop removal */
        local_sink += i;
        sum += i;
        dummy_side_effect(i);
    }
    
    return sum + local_sink;
}

/* Function B: while loop with unsigned counter */
__attribute__((optimize("O2")))
unsigned int test_while_loop_decrement(unsigned int n) {
    unsigned int checksum = 0;
    
    /* Another decrement-and-compare pattern: while(n--) */
    while (n--) {
        checksum ^= n;  /* Non-trivial computation */
        extern_array[n % 1000] = n;  /* External side effect */
    }
    
    return checksum;
}

/* Function C: Nested loops with inner decrement pattern */
__attribute__((optimize("O2")))
int test_nested_loops(int outer, int inner) {
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop uses decrement-and-compare pattern */
        for (int i = inner; i-- > 0;) {
            total += o * i;
            extern_sink++;  /* Volatile side effect */
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant folding) */
__attribute__((optimize("O2")))
int test_param_counter(int count) {
    int result = 0;
    
    /* Hide loop bound behind volatile to prevent optimization */
    volatile int hidden_bound = count;
    
    /* Decrement pattern with parameterized bound */
    for (int i = hidden_bound; i-- > 0;) {
        result += (i * 3) / 2;
        dummy_side_effect(result);
    }
    
    return result;
}

/* Function E: Mixed decrement patterns with different types */
__attribute__((optimize("O2")))
long test_mixed_patterns(short s_count, char c_count) {
    long accumulator = 0;
    
    /* Pattern 1: short counter */
    for (short s = s_count; s-- > 0;) {
        accumulator += s;
        extern_array[accumulator % 1000] = s;
    }
    
    /* Pattern 2: char counter */
    unsigned char uc = c_count;
    while (uc--) {
        accumulator -= uc;
        dummy_side_effect(uc);
    }
    
    return accumulator;
}

/* Function F: Complex expression in loop body, simple decrement pattern */
__attribute__((optimize("O2")))
int test_complex_body_decrement(int n) {
    int fib1 = 0, fib2 = 1;
    
    /* Simple decrement pattern with Fibonacci computation in body */
    for (int i = n; i-- > 0;) {
        int next = fib1 + fib2;
        fib1 = fib2;
        fib2 = next;
        extern_sink = fib1;  /* Volatile store */
    }
    
    return fib1;
}

/* Main driver that calls all test functions */
int main(void) {
    int final_checksum = 0;
    
    /* Initialize volatile bound */
    volatile_bound = 100;
    
    /* Test 1: Basic for loop */
    int result1 = test_for_loop_decrement(1000);
    printf("Test 1 result: %d\n", result1);
    final_checksum ^= result1;
    
    /* Test 2: While loop with unsigned */
    unsigned int result2 = test_while_loop_decrement(500);
    printf("Test 2 result: %u\n", result2);
    final_checksum ^= (int)result2;
    
    /* Test 3: Nested loops */
    int result3 = test_nested_loops(10, 100);
    printf("Test 3 result: %d\n", result3);
    final_checksum ^= result3;
    
    /* Test 4: Parameter counter */
    int result4 = test_param_counter(250);
    printf("Test 4 result: %d\n", result4);
    final_checksum ^= result4;
    
    /* Test 5: Mixed patterns */
    long result5 = test_mixed_patterns(50, 30);
    printf("Test 5 result: %ld\n", result5);
    final_checksum ^= (int)result5;
    
    /* Test 6: Complex body */
    int result6 = test_complex_body_decrement(20);
    printf("Test 6 result: %d\n", result6);
    final_checksum ^= result6;
    
    printf("Final checksum: %d\n", final_checksum);
    
    /* Additional verification */
    printf("External sink value: %d\n", extern_sink);
    
    return final_checksum != 0 ? 0 : 1;
}
