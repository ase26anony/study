/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Dummy function with side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    /* Use volatile to ensure side effect isn't optimized away */
    static volatile int sink = 0;
    sink += value;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_decrement(int iterations) {
    volatile int checksum = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        checksum += i;
        dummy_side_effect(i);
    }
    
    return checksum;
}

/* Function B: while loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int n) {
    volatile int checksum = 0;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        checksum += (int)n;
        extern_array[n % 1000] = n;  /* External side effect */
    }
    
    return checksum;
}

/* Function C: Nested loops with inner decrement pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop uses decrement-and-compare pattern */
        for (int i = inner; i-- > 0;) {
            total += o * i;
            dummy_side_effect(o + i);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    volatile int result = 0;
    
    /* Counter from parameter ensures runtime value */
    for (int i = count; i-- > 0;) {
        result += i * 2;
        extern_array[i % 500] = result;
    }
    
    return result;
}

/* Function E: Do-while loop that should also match the pattern */
int __attribute__((optimize("O2"))) test_do_while_decrement(int limit) {
    volatile int sum = 0;
    int i = limit;
    
    if (i <= 0) return 0;
    
    do {
        sum += i;
        dummy_side_effect(i);
    } while (i-- > 1);  /* Decrement and compare at end */
    
    return sum;
}

/* Function F: Loop with volatile bound to prevent compile-time folding */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 1000;
    volatile int accumulator = 0;
    
    /* Bound is volatile, can't be constant folded */
    for (int j = bound; j-- > 0;) {
        accumulator += j * 3;
        extern_array[j % 100] = j;
    }
    
    return accumulator;
}

/* Function G: Mixed decrement patterns in same function */
int __attribute__((optimize("O2"))) test_mixed_patterns(int a, int b) {
    volatile int total = 0;
    
    /* First loop: for with decrement */
    for (int i = a; i-- > 0;) {
        total += i;
    }
    
    /* Second loop: while with decrement */
    unsigned int u = (unsigned int)b;
    while (u--) {
        total -= (int)u;
        dummy_side_effect(u);
    }
    
    return total;
}

/* Main driver function */
int main(void) {
    int final_checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test different loop patterns */
    final_checksum += test_for_loop_decrement(100);
    printf("  test_for_loop_decrement completed\n");
    
    final_checksum += test_while_loop_decrement(200);
    printf("  test_while_loop_decrement completed\n");
    
    final_checksum += test_nested_loops(10, 50);
    printf("  test_nested_loops completed\n");
    
    final_checksum += test_param_counter(150);
    printf("  test_param_counter completed\n");
    
    final_checksum += test_do_while_decrement(75);
    printf("  test_do_while_decrement completed\n");
    
    final_checksum += test_volatile_bound();
    printf("  test_volatile_bound completed\n");
    
    final_checksum += test_mixed_patterns(60, 80);
    printf("  test_mixed_patterns completed\n");
    
    printf("\nFinal checksum: %d\n", final_checksum);
    printf("All loops executed successfully.\n");
    
    return 0;
}

/* Define the external array to avoid linker errors */
int extern_array[10000] = {0};
