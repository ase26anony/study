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

/* Function B: While loop with unsigned counter */
int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int count) {
    volatile int checksum = 0;
    unsigned int n = count;
    
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
    int c = count;
    
    /* Use volatile to hide loop bound from optimizer */
    volatile int bound = count;
    c = bound;
    
    /* Decrement pattern with parameterized counter */
    while (c--) {
        result += c * 2;
        extern_array[c % 1000] = result;
    }
    
    return result;
}

/* Function E: Do-while with explicit decrement (alternative pattern) */
int __attribute__((optimize("O2"))) test_do_while_decrement(int limit) {
    volatile int sum = 0;
    int counter = limit;
    
    if (counter <= 0) return 0;
    
    do {
        sum += counter;
        dummy_side_effect(counter);
    } while (--counter > 0);
    
    return sum;
}

/* Function F: Loop with volatile counter (ensures memory operations) */
int __attribute__((optimize("O2"))) test_volatile_counter(int base) {
    volatile int vcounter = base;
    volatile int accum = 0;
    
    /* Force memory reads/writes in the loop */
    while (vcounter--) {
        accum += vcounter;
        /* Array access ensures side effect */
        if (vcounter < 1000) {
            extern_array[vcounter] = accum;
        }
    }
    
    return accum;
}

/* Main driver that calls all test functions */
int main(void) {
    int final_checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test with different iteration counts to exercise various paths */
    final_checksum += test_for_loop_decrement(1000);
    printf("Test A complete\n");
    
    final_checksum += test_while_loop_decrement(500);
    printf("Test B complete\n");
    
    final_checksum += test_nested_loops(10, 100);
    printf("Test C complete\n");
    
    final_checksum += test_param_counter(750);
    printf("Test D complete\n");
    
    final_checksum += test_do_while_decrement(300);
    printf("Test E complete\n");
    
    final_checksum += test_volatile_counter(200);
    printf("Test F complete\n");
    
    printf("Final checksum: %d\n", final_checksum);
    printf("All loops executed successfully.\n");
    
    /* Verify by computing expected value */
    int expected = 0;
    for (int i = 1000; i-- > 0;) expected += i;
    for (unsigned int n = 500; n-- > 0;) expected += n;
    for (int o = 0; o < 10; o++)
        for (int i = 100; i-- > 0;) expected += o * i;
    for (int c = 750; c-- > 0;) expected += c * 2;
    for (int c = 300; c > 0; c--) expected += c;
    for (int v = 200; v-- > 0;) expected += v;
    
    printf("Expected checksum: %d\n", expected);
    printf("Test %s\n", (final_checksum == expected) ? "PASSED" : "FAILED");
    
    return (final_checksum == expected) ? 0 : 1;
}

/* Define the external array */
int extern_array[10000] = {0};
