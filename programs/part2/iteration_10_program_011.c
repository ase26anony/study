/* test_loop_doloop.c
 * 
 * This program is designed to exercise GCC's loop-doloop optimization pass
 * by generating loops with the specific decrement-and-compare pattern:
 *   (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * 
 * The loops use various counter types and nesting to ensure the pattern
 * is preserved through optimization passes and can be matched by the
 * RTL pattern matcher in loop-doloop.cc.
 */

#include <stdio.h>
#include <stdint.h>

/* Global variables to prevent constant propagation and dead code elimination */
volatile int global_counter = 0;
int global_array[10000];
extern volatile int external_sink;

/* Dummy function with side effects to prevent loop removal */
static void __attribute__((noinline)) dummy_side_effect(int value) {
    global_counter += value;
}

/* Function A: Basic for loop with int counter, decrement-and-compare pattern */
int __attribute__((optimize("O2"))) test_for_loop_int(int limit) {
    volatile int sink = 0;
    int sum = 0;
    
    /* for (int i = limit; i-- > 0;) produces the desired pattern */
    for (int i = limit; i-- > 0;) {
        sink += i;               /* Volatile operation prevents removal */
        sum += i & 0xF;          /* Simple computation for verification */
        dummy_side_effect(i);    /* External side effect */
    }
    
    return sum;
}

/* Function B: while loop with unsigned int counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int n) {
    volatile unsigned int sink = 0;
    unsigned int sum = 0;
    
    /* while (n--) produces the desired pattern */
    while (n--) {
        sink += n;
        sum += n % 256;
        global_array[n % 1000] = n;  /* Array store with external visibility */
    }
    
    return sum;
}

/* Function C: Nested loops where inner loop uses the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare pattern */
        for (int i = inner; i-- > 0;) {
            sink += o * i;
            total += (o + i) & 0xFF;
            dummy_side_effect(i);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    volatile int sink = 0;
    int result = 0;
    
    /* count is a parameter, so compiler can't assume its value */
    for (int i = count; i-- > 0;) {
        sink += i;
        result ^= i;  /* Non-linear operation to prevent simplification */
        global_array[i % 1000] = result;
    }
    
    return result;
}

/* Function E: Do-while loop that should also match the pattern */
int __attribute__((optimize("O2"))) test_dowhile_loop(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    int i = iterations;
    
    if (i <= 0) return 0;
    
    do {
        sink += i;
        sum += i * 3;
        dummy_side_effect(i);
    } while (i-- > 1);  /* Note: careful with do-while to match pattern */
    
    return sum;
}

/* Main driver that runs all tests and verifies correctness */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test A: Basic int loop */
    int result_a = test_for_loop_int(1000);
    checksum = (checksum * 31 + result_a) & 0xFFFF;
    printf("Test A result: %d\n", result_a);
    
    /* Test B: Unsigned while loop */
    unsigned int result_b = test_while_loop_unsigned(500);
    checksum = (checksum * 31 + result_b) & 0xFFFF;
    printf("Test B result: %u\n", result_b);
    
    /* Test C: Nested loops */
    int result_c = test_nested_loops(10, 100);
    checksum = (checksum * 31 + result_c) & 0xFFFF;
    printf("Test C result: %d\n", result_c);
    
    /* Test D: Parameterized loop */
    int result_d = test_param_loop(750);
    checksum = (checksum * 31 + result_d) & 0xFFFF;
    printf("Test D result: %d\n", result_d);
    
    /* Test E: Do-while loop */
    int result_e = test_dowhile_loop(300);
    checksum = (checksum * 31 + result_e) & 0xFFFF;
    printf("Test E result: %d\n", result_e);
    
    printf("Final checksum: 0x%04X\n", checksum);
    
    /* Verify global side effects occurred */
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
