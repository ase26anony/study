/* loop-doloop-coverage.c
 * Test program to cover decrement-and-compare pattern matching in GCC's loop-doloop pass.
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 -o test loop-doloop-coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* External array to prevent dead code elimination */
volatile int extern_array[10000];
volatile int extern_index = 0;

/* Dummy function with side effects to prevent loop removal */
static void __attribute__((noinline)) side_effect(int value) {
    extern_array[extern_index++ % 10000] = value;
}

/* Function A: Basic for loop with int counter using post-decrement pattern */
int __attribute__((optimize("O2"))) test_for_loop_int(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    
    /* for (int i = iterations; i-- > 0;) produces the desired pattern */
    for (int i = iterations; i-- > 0;) {
        sink += i;           /* Volatile operation prevents optimization */
        sum += i & 0xFF;     /* Simple computation for verification */
        side_effect(i);      /* External side effect */
    }
    
    return sum & 0xFFFF;
}

/* Function B: while loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int n) {
    volatile int sink = 0;
    int sum = 0;
    
    /* while (n--) produces the desired pattern */
    while (n--) {
        sink += n;
        sum += (n * 3) & 0xFF;
        side_effect(n);
    }
    
    return sum & 0xFFFF;
}

/* Function C: Nested loops with inner loop using the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int sink = 0;
    int sum = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare pattern */
        for (int i = inner; i-- > 0;) {
            sink += o * i;
            sum += (o + i) & 0xFF;
            side_effect(o * 1000 + i);
        }
    }
    
    return sum & 0xFFFF;
}

/* Function D: Loop with counter as function parameter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    volatile int sink = 0;
    int sum = 0;
    int local_count = count;  /* Copy to local to ensure decrement happens */
    
    /* Use local_count to prevent compiler from knowing exact value */
    while (local_count--) {
        sink += local_count;
        sum += (local_count * 5) & 0xFF;
        side_effect(local_count);
    }
    
    return sum & 0xFFFF;
}

/* Function E: Loop with volatile bound to prevent compile-time evaluation */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 100;  /* Volatile prevents constant propagation */
    volatile int sink = 0;
    int sum = 0;
    int i = bound;
    
    /* Decrement pattern with volatile-influenced bound */
    while (i--) {
        sink += i;
        sum += i & 0xFF;
        side_effect(i);
    }
    
    return sum & 0xFFFF;
}

/* Function F: Do-while loop that should also match the pattern */
int __attribute__((optimize("O2"))) test_dowhile_loop(int count) {
    volatile int sink = 0;
    int sum = 0;
    int i = count;
    
    if (i <= 0) return 0;
    
    do {
        sink += i;
        sum += i & 0xFF;
        side_effect(i);
    } while (i-- > 1);  /* Note: careful with boundary to avoid underflow */
    
    return sum & 0xFFFF;
}

/* Main driver that calls all test functions and verifies results */
int main(void) {
    int checksum = 0;
    
    printf("Running loop-doloop pattern tests...\n");
    
    /* Test 1: Basic for loop with int counter */
    checksum ^= test_for_loop_int(1000);
    printf("Test 1 complete\n");
    
    /* Test 2: While loop with unsigned counter */
    checksum ^= test_while_loop_unsigned(500);
    printf("Test 2 complete\n");
    
    /* Test 3: Nested loops */
    checksum ^= test_nested_loops(10, 100);
    printf("Test 3 complete\n");
    
    /* Test 4: Parameter-based loop */
    checksum ^= test_param_loop(750);
    printf("Test 4 complete\n");
    
    /* Test 5: Volatile bound loop */
    checksum ^= test_volatile_bound();
    printf("Test 5 complete\n");
    
    /* Test 6: Do-while loop */
    checksum ^= test_dowhile_loop(300);
    printf("Test 6 complete\n");
    
    /* Final checksum to ensure all loops executed */
    printf("Final checksum: 0x%04X\n", checksum & 0xFFFF);
    
    /* Verify some side effects occurred */
    printf("Side effects recorded: %d\n", extern_index);
    
    return 0;
}
