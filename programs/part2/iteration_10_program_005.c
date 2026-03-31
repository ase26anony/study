/* test_loop_doloop.c - Target for GCC loop-doloop.cc coverage */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of dummy function */
static void __attribute__((noinline)) dummy_side_effect(int value) {
    /* Use volatile to prevent optimization */
    static volatile int sink;
    sink += value;
}

/* External array to prevent dead code elimination */
extern int extern_array[1000];

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_decrement(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sink += i;          /* Volatile side effect */
        sum += i & 0xFF;    /* Simple computation */
        extern_array[i & 999] = i; /* External side effect */
    }
    
    dummy_side_effect(sum);
    return sum;
}

/* Function B: while loop with unsigned counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int n) {
    volatile unsigned int sink = 0;
    unsigned int sum = 0;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        sink += n;
        sum += n * 3;
        dummy_side_effect(n);
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
            total += (o * 100) + i;
            extern_array[(o * i) % 1000] = total;
        }
    }
    
    dummy_side_effect(total);
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    volatile int sink = 0;
    int result = 0;
    
    /* Counter from parameter - compiler can't assume value */
    for (int i = count; i-- > 0;) {
        sink += i;
        result ^= i;  /* Non-linear to prevent simplification */
        dummy_side_effect(result);
    }
    
    return result;
}

/* Function E: Do-while with explicit decrement (alternative pattern) */
int __attribute__((optimize("O2"))) test_dowhile_decrement(int limit) {
    volatile int sink = 0;
    int i = limit;
    int sum = 0;
    
    if (i <= 0) return 0;
    
    do {
        sink += i;
        sum += i * i;
        extern_array[i % 1000] = sum;
    } while (--i > 0);  /* Decrement and compare */
    
    dummy_side_effect(sum);
    return sum;
}

/* Function F: Mixed types to test different register sizes */
long __attribute__((optimize("O2"))) test_long_counter(long n) {
    volatile long sink = 0;
    long sum = 0;
    
    /* Long counter with decrement pattern */
    for (long i = n; i-- > 0;) {
        sink += i;
        sum += i % 37;
        dummy_side_effect((int)i);
    }
    
    return sum;
}

/* Global volatile to hide loop bounds from constant propagation */
volatile int hidden_bound = 100;

/* Function G: Loop bound from volatile variable */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int sink = 0;
    int bound = hidden_bound;  /* Volatile read */
    int sum = 0;
    
    /* Bound not known at compile time */
    for (int i = bound; i-- > 0;) {
        sink += i;
        sum += (i * 2) + 1;
        extern_array[sum % 1000] = i;
    }
    
    return sum;
}

/* Main driver with checksum verification */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test 1: Basic for loop */
    checksum += test_for_loop_decrement(100);
    printf("Test 1 complete\n");
    
    /* Test 2: While loop */
    checksum += test_while_loop_decrement(50);
    printf("Test 2 complete\n");
    
    /* Test 3: Nested loops */
    checksum += test_nested_loops(10, 20);
    printf("Test 3 complete\n");
    
    /* Test 4: Parameter counter */
    checksum += test_param_counter(75);
    printf("Test 4 complete\n");
    
    /* Test 5: Do-while */
    checksum += test_dowhile_decrement(60);
    printf("Test 5 complete\n");
    
    /* Test 6: Long counter */
    checksum += test_long_counter(30);
    printf("Test 6 complete\n");
    
    /* Test 7: Volatile bound */
    checksum += test_volatile_bound();
    printf("Test 7 complete\n");
    
    printf("Final checksum: %d\n", checksum);
    printf("All loops executed successfully.\n");
    
    return 0;
}

/* Define the external array */
int extern_array[1000] = {0};
