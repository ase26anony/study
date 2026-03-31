/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Dummy function with side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    /* Use volatile to prevent optimization */
    static volatile int sink = 0;
    sink += value;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_decrement(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Classic decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sink += i;           /* Volatile access prevents elimination */
        sum += i & 0xFF;     /* Simple computation */
        dummy_side_effect(i); /* External side effect */
    }
    
    return sum;
}

/* Function B: While loop with unsigned counter */
int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int n) {
    volatile int sink = 0;
    int sum = 0;
    
    /* while(n--) pattern - should generate decrement-and-compare */
    while (n--) {
        sink += n;
        sum += (n % 256);
        extern_array[n % 1000] = n; /* External memory store */
    }
    
    return sum;
}

/* Function C: Nested loops with inner decrement pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop uses decrement-and-compare pattern */
        int i = inner;
        while (i--) {
            sink += o * i;
            total += (o + i) & 0xFF;
            dummy_side_effect(o + i);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    volatile int sink = 0;
    int result = 0;
    
    /* Counter from parameter - compiler can't assume value */
    for (int i = count; i-- > 0;) {
        sink += i;
        result ^= i;  /* Non-linear operation prevents simplification */
        extern_array[i % 100] = result;
    }
    
    return result;
}

/* Function E: Mixed counter types with volatile bound */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 1000; /* Volatile prevents constant folding */
    int sum = 0;
    
    /* Loop with volatile-controlled iterations */
    int i = bound;
    while (i--) {
        sum += i;
        dummy_side_effect(sum);
    }
    
    return sum;
}

/* Function F: Do-while style that should still match the pattern */
int __attribute__((optimize("O2"))) test_do_while_pattern(int n) {
    volatile int sink = 0;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    /* Manual do-while with decrement-and-compare */
    int i = n;
    do {
        sink += i;
        sum += i * 2;
        dummy_side_effect(i);
    } while (i-- > 1);  /* Note: careful with boundary */
    
    return sum;
}

/* Main driver with checksum verification */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test with various iteration counts to explore different paths */
    checksum += test_for_loop_decrement(1000);
    checksum += test_while_loop_decrement(500);
    checksum += test_nested_loops(10, 100);
    checksum += test_param_counter(750);
    checksum += test_volatile_bound();
    checksum += test_do_while_pattern(300);
    
    /* Add some variation with different values */
    checksum += test_for_loop_decrement(1);   /* Edge case */
    checksum += test_while_loop_decrement(0); /* Edge case */
    checksum += test_nested_loops(1, 1);      /* Minimal case */
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum != 0, loops executed successfully.\n");
    
    return 0;
}

/* Define the external array */
int extern_array[10000] = {0};
