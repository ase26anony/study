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
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sink = i;           /* Volatile store prevents elimination */
        sum += i & 0xFF;    /* Simple computation */
        dummy_side_effect(i); /* External side effect */
    }
    
    return sum;
}

/* Function B: While loop with unsigned counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int n) {
    volatile unsigned int sink = 0;
    unsigned int checksum = 0;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        sink = n;
        checksum ^= n;      /* Non-trivial computation */
        extern_array[n % 1000] = n; /* External memory store */
    }
    
    return checksum;
}

/* Function C: Nested loops with inner decrement pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare tail */
        for (int i = inner; i-- > 0;) {
            sink = o * 1000 + i;
            total += (o * i) & 0xFF;
            dummy_side_effect(i);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    volatile int sink = 0;
    int result = 0;
    
    /* Counter from parameter ensures runtime value */
    while (count--) {
        sink = count;
        result += (count * 13) % 256;
        extern_array[count % 500] = result;
    }
    
    return result;
}

/* Function E: Mixed decrement patterns with volatile bound */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 100;  /* Volatile prevents compile-time folding */
    int sum = 0;
    int i = bound;
    
    /* Manual while loop with explicit decrement */
    while (1) {
        if (i-- <= 0) break;  /* Alternative decrement-and-compare pattern */
        sum += i & 0xF;
        dummy_side_effect(sum);
    }
    
    return sum;
}

/* Function F: Do-while with pre-decrement (should also match pattern) */
int __attribute__((optimize("O2"))) test_do_while_decrement(int start) {
    volatile int sink = 0;
    int acc = 0;
    int counter = start;
    
    if (counter <= 0) return 0;
    
    do {
        sink = counter;
        acc = (acc * 31 + counter) & 0xFFFF;
        extern_array[acc % 1000] = counter;
    } while (--counter > 0);  /* Pre-decrement and compare */
    
    return acc;
}

/* Global array definition */
int extern_array[10000] = {0};

/* Main driver with verification */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test each function with different parameters */
    checksum += test_for_loop_decrement(1000);
    printf("Test A complete\n");
    
    checksum += test_while_loop_decrement(500);
    printf("Test B complete\n");
    
    checksum += test_nested_loops(10, 100);
    printf("Test C complete\n");
    
    checksum += test_param_counter(750);
    printf("Test D complete\n");
    
    checksum += test_volatile_bound();
    printf("Test E complete\n");
    
    checksum += test_do_while_decrement(300);
    printf("Test F complete\n");
    
    /* Verify array was touched */
    int array_sum = 0;
    for (int i = 0; i < 1000; i++) {
        array_sum += extern_array[i];
    }
    checksum += array_sum;
    
    printf("Final checksum: %d\n", checksum);
    printf("All loops executed successfully.\n");
    
    return 0;
}
