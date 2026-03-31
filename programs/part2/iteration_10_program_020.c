/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Dummy function with side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    /* Use volatile to ensure side effect isn't optimized away */
    volatile static int sink = 0;
    sink += value;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_int(int iterations) {
    volatile int result = 0;
    
    /* Classic decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        /* Simple side effect that can't be optimized away */
        result += i;
        dummy_side_effect(i);
    }
    
    return result;
}

/* Function B: While loop with unsigned int counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int n) {
    volatile unsigned int sum = 0;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        sum += n;
        /* Store to external array to create side effect */
        if (n < 10000) extern_array[n] = (int)n;
    }
    
    return sum;
}

/* Function C: Nested loops with inner loop using the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop uses decrement-and-compare pattern */
        int j = inner;
        while (j--) {
            total += o * j;
            dummy_side_effect(j);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    volatile int checksum = 0;
    
    /* Use parameter directly in decrement-and-compare */
    for (; count-- > 0;) {
        checksum ^= count;  /* Non-linear operation to prevent optimization */
        if (count < 10000) extern_array[count] = checksum;
    }
    
    return checksum;
}

/* Function E: Loop with volatile bound to prevent compile-time evaluation */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 1000;
    volatile int accumulator = 0;
    
    /* Bound is volatile, can't be optimized at compile time */
    int i = bound;
    while (i--) {
        accumulator += i * 3;
        dummy_side_effect(accumulator);
    }
    
    return accumulator;
}

/* Function F: Do-while loop that should also match the pattern */
int __attribute__((optimize("O2"))) test_do_while_pattern(int limit) {
    volatile int counter = 0;
    
    if (limit <= 0) return 0;
    
    int i = limit;
    do {
        counter += i;
        dummy_side_effect(i);
    } while (i-- > 1);  /* Decrement and compare at the end */
    
    return counter;
}

/* Global array definition */
int extern_array[10000] = {0};

int main(void) {
    int final_checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test Function A */
    int result_a = test_for_loop_int(1000);
    final_checksum ^= result_a;
    printf("Test A (for loop int): %d\n", result_a);
    
    /* Test Function B */
    unsigned int result_b = test_while_loop_unsigned(500);
    final_checksum ^= (int)result_b;
    printf("Test B (while loop unsigned): %u\n", result_b);
    
    /* Test Function C */
    int result_c = test_nested_loops(10, 100);
    final_checksum ^= result_c;
    printf("Test C (nested loops): %d\n", result_c);
    
    /* Test Function D */
    int result_d = test_param_counter(750);
    final_checksum ^= result_d;
    printf("Test D (parameter counter): %d\n", result_d);
    
    /* Test Function E */
    int result_e = test_volatile_bound();
    final_checksum ^= result_e;
    printf("Test E (volatile bound): %d\n", result_e);
    
    /* Test Function F */
    int result_f = test_do_while_pattern(300);
    final_checksum ^= result_f;
    printf("Test F (do-while pattern): %d\n", result_f);
    
    /* Verify array was touched */
    int array_sum = 0;
    for (int i = 0; i < 10000; i++) {
        array_sum += extern_array[i];
    }
    final_checksum ^= array_sum;
    
    printf("Final checksum: %d\n", final_checksum);
    printf("Array checksum: %d\n", array_sum);
    
    return 0;
}
