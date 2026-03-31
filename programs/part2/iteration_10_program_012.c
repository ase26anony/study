/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent optimization */
extern int extern_array[10000];

/* Dummy function with side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    /* Use volatile to ensure side effect isn't optimized away */
    static volatile int sink = 0;
    sink += value;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_decrement(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sink = i;           /* Volatile store prevents dead code elimination */
        sum += i & 0xFF;    /* Simple computation */
        dummy_side_effect(i);
    }
    
    return sum;
}

/* Function B: while loop with unsigned int counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int n) {
    volatile unsigned int sink = 0;
    unsigned int checksum = 0;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        sink = n;
        checksum ^= n;      /* Non-linear operation to prevent optimization */
        extern_array[n % 1000] = n;  /* External side effect */
    }
    
    return checksum;
}

/* Function C: Nested loops with inner decrement pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop uses decrement-and-compare pattern */
        int i = inner;
        while (i--) {
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
    for (int i = count; i-- > 0;) {
        sink = i;
        result += i * i;
        extern_array[i % 1000] = result;
    }
    
    return result;
}

/* Function E: Multiple decrement patterns in same function */
int __attribute__((optimize("O2"))) test_multiple_patterns(void) {
    volatile int sink = 0;
    int sum = 0;
    
    /* First loop */
    for (int i = 100; i-- > 0;) {
        sink = i;
        sum += i;
    }
    
    /* Second loop with different counter */
    unsigned int j = 50;
    while (j--) {
        sink = j;
        sum += j * 2;
        dummy_side_effect(j);
    }
    
    return sum;
}

/* Function F: Loop with volatile bound (prevents compile-time folding) */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 200;  /* Volatile prevents constant propagation */
    volatile int sink = 0;
    int acc = 0;
    
    int i = bound;
    while (i--) {
        sink = i;
        acc += i % 37;
        extern_array[acc % 1000] = i;
    }
    
    return acc;
}

/* Global array definition */
int extern_array[10000] = {0};

int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern coverage...\n");
    
    /* Call each test function and accumulate results */
    checksum += test_for_loop_decrement(1000);
    printf("Test A complete\n");
    
    checksum += test_while_loop_decrement(500);
    printf("Test B complete\n");
    
    checksum += test_nested_loops(10, 100);
    printf("Test C complete\n");
    
    checksum += test_param_counter(300);
    printf("Test D complete\n");
    
    checksum += test_multiple_patterns();
    printf("Test E complete\n");
    
    checksum += test_volatile_bound();
    printf("Test F complete\n");
    
    /* Verify array was touched */
    int array_sum = 0;
    for (int i = 0; i < 1000; i++) {
        array_sum += extern_array[i];
    }
    checksum += array_sum;
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum != 0, loops executed successfully.\n");
    
    return 0;
}
