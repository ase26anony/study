/* test_loop_doloop.c
 * Program to exercise GCC's loop-doloop pass decrement-and-compare pattern matching
 * Pattern: (set (reg:CC) (compare (plus (reg) -1) (const0)))
 */

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
    volatile int sink = 0;
    int sum = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sink += i;           /* Volatile operation prevents elimination */
        sum += i & 0xFF;     /* Simple computation for verification */
    }
    
    return sum;
}

/* Function B: while loop with unsigned int counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int n) {
    volatile unsigned int sink = 0;
    unsigned int sum = 0;
    
    /* while(n--) pattern - should generate decrement-and-compare */
    while (n--) {
        sink += n;
        sum += n % 256;
        dummy_side_effect(n);  /* External call prevents optimization */
    }
    
    return sum;
}

/* Function C: Nested loops with inner decrement pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare pattern */
        for (int i = inner; i-- > 0;) {
            sink += o * i;
            total += (o + i) & 0xFF;
            extern_array[total % 10000] = o + i;  /* External array access */
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
        sink += i;
        result ^= i;  /* Non-linear operation prevents simplification */
        dummy_side_effect(result);
    }
    
    return result;
}

/* Function E: Do-while loop with explicit decrement (alternative pattern) */
int __attribute__((optimize("O2"))) test_dowhile_decrement(int limit) {
    volatile int sink = 0;
    int i = limit;
    int checksum = 0;
    
    if (i > 0) {
        do {
            sink += i;
            checksum += i * 3;
            dummy_side_effect(checksum);
        } while (--i > 0);  /* Pre-decrement with compare */
    }
    
    return checksum;
}

/* Function F: Loop with volatile bound (prevents compile-time folding) */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 1000;  /* Volatile prevents constant propagation */
    volatile int sink = 0;
    int sum = 0;
    
    for (int i = bound; i-- > 0;) {
        sink += i;
        sum += i % 37;
    }
    
    return sum;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test 1: Basic for loop with decrement */
    checksum += test_for_loop_decrement(1000);
    printf("Test 1 complete\n");
    
    /* Test 2: While loop with unsigned decrement */
    checksum += test_while_loop_decrement(500);
    printf("Test 2 complete\n");
    
    /* Test 3: Nested loops */
    checksum += test_nested_loops(10, 100);
    printf("Test 3 complete\n");
    
    /* Test 4: Parameterized counter */
    checksum += test_param_counter(750);
    printf("Test 4 complete\n");
    
    /* Test 5: Do-while with pre-decrement */
    checksum += test_dowhile_decrement(300);
    printf("Test 5 complete\n");
    
    /* Test 6: Volatile loop bound */
    checksum += test_volatile_bound();
    printf("Test 6 complete\n");
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum != 0, loops executed successfully\n");
    
    return 0;
}

/* Define the external array */
int extern_array[10000] = {0};
