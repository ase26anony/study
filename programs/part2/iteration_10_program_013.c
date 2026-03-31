/* test_loop_doloop.c - Test program for GCC loop-doloop pass coverage */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of dummy function */
__attribute__((noinline)) 
static void dummy_side_effect(int value) {
    /* Empty but prevents dead code elimination */
    (void)value;
}

/* External array to prevent optimization */
extern volatile int external_buffer[1024];

/* Function A: Basic for loop with int counter */
__attribute__((optimize("O2")))
static int test_basic_for_loop(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sink += i;           /* Volatile operation prevents elimination */
        sum += i & 0xFF;     /* Simple computation for verification */
        dummy_side_effect(i); /* External side effect */
    }
    
    return sum;
}

/* Function B: while loop with unsigned counter */
__attribute__((optimize("O2")))
static int test_while_loop(unsigned int count) {
    volatile int sink = 0;
    int sum = 0;
    unsigned int n = count;
    
    /* Decrement-and-compare in while condition */
    while (n--) {
        sink += n;
        sum += (int)(n % 256);
        if (external_buffer) { /* Reference external to prevent opt */
            dummy_side_effect(n);
        }
    }
    
    return sum;
}

/* Function C: Nested loops with inner decrement pattern */
__attribute__((optimize("O2")))
static int test_nested_loops(int outer, int inner) {
    volatile int sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop uses decrement-and-compare pattern */
        for (int i = inner; i-- > 0;) {
            sink += o * i;
            total += (o + i) & 0xFF;
            dummy_side_effect(i);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
__attribute__((optimize("O2")))
static int test_param_loop(int count) {
    volatile int sink = 0;
    int sum = 0;
    int c = count;
    
    /* Use parameter to prevent compile-time folding */
    while (c--) {
        sink += c;
        sum += c * 2;
        dummy_side_effect(c);
        
        /* Small conditional to prevent if-conversion but preserve loop */
        if (c & 1) {
            sum += 1;
        }
    }
    
    return sum;
}

/* Function E: Do-while loop that should also match the pattern */
__attribute__((optimize("O2")))
static int test_dowhile_loop(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    int i = iterations;
    
    if (i <= 0) return 0;
    
    do {
        sink += i;
        sum += i & 0x7F;
        dummy_side_effect(i);
    } while (i-- > 1);  /* Decrement and compare at end */
    
    return sum;
}

/* Main driver that ensures all loops execute */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test with different values to avoid constant folding */
    checksum += test_basic_for_loop(1000);
    checksum += test_while_loop(500);
    checksum += test_nested_loops(10, 100);
    checksum += test_param_loop(750);
    checksum += test_dowhile_loop(300);
    
    /* Additional tests with volatile bounds to prevent optimization */
    volatile int dynamic_bound = 200;
    checksum += test_basic_for_loop(dynamic_bound);
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum != 0, loops executed successfully.\n");
    
    return 0;
}
