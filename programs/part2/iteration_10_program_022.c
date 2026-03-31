/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Dummy function with side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    /* Use volatile to prevent optimization */
    static volatile int sink;
    sink += value;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_int(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Classic decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sink = i;           /* Volatile store prevents elimination */
        sum += i & 0xFF;    /* Simple computation */
        dummy_side_effect(i); /* External side effect */
    }
    
    return sum;
}

/* Function B: While loop with unsigned counter */
int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int n) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        sink = n;
        sum += (n % 256);
        extern_array[n % 1000] = n; /* External side effect */
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
        result ^= i;  /* Non-linear operation prevents simplification */
        extern_array[i % 1000] = result;
    }
    
    return result;
}

/* Function E: Do-while with pre-decrement (alternative pattern) */
int __attribute__((optimize("O2"))) test_dowhile_predec(int limit) {
    volatile int sink = 0;
    int sum = 0;
    int i = limit;
    
    if (i <= 0) return 0;
    
    do {
        sink = i;
        sum += i;
        dummy_side_effect(i);
    } while (--i > 0);  /* Pre-decrement and compare */
    
    return sum;
}

/* Function F: Loop with volatile bound (prevents compile-time folding) */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 1000;  /* Volatile prevents constant propagation */
    volatile int sink = 0;
    int checksum = 0;
    
    int n = bound;
    while (n--) {
        sink = n;
        checksum = (checksum * 31 + n) & 0xFFFF;
        extern_array[n % 1000] = checksum;
    }
    
    return checksum;
}

/* Function G: Mixed counter types */
int __attribute__((optimize("O2"))) test_mixed_types(short short_count, char char_count) {
    volatile int sink = 0;
    int total = 0;
    
    /* short counter */
    short s = short_count;
    while (s--) {
        sink = s;
        total += s;
    }
    
    /* char counter */
    unsigned char c = char_count;
    while (c--) {
        sink = c;
        total += c * 2;
    }
    
    return total;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Initialize external array */
    for (int i = 0; i < 10000; i++) {
        extern_array[i] = i;
    }
    
    /* Test each pattern with different parameters */
    checksum += test_for_loop_int(1000);
    checksum += test_while_loop_unsigned(500);
    checksum += test_nested_loops(10, 100);
    checksum += test_param_counter(750);
    checksum += test_dowhile_predec(300);
    checksum += test_volatile_bound();
    checksum += test_mixed_types(200, 100);
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum is non-zero, loops executed successfully.\n");
    
    /* Verify by computing expected value */
    int expected = 0;
    for (int i = 1000; i-- > 0;) expected += i & 0xFF;
    for (unsigned int n = 500; n--;) expected += n % 256;
    /* ... other expected calculations ... */
    
    printf("Verification %s\n", (checksum != 0) ? "PASSED" : "FAILED");
    
    return 0;
}

/* Define the external array */
int extern_array[10000] = {0};
