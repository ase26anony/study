/* test-loop-doloop.c
 * Designed to trigger the specific RTL pattern in loop-doloop.cc lines 136-150
 * Pattern: SET with COMPARE of (PLUS reg -1) against const0_rtx
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variable to prevent dead code elimination */
volatile int global_sum = 0;

/* Test 1: do-while loop with pre-decrement */
int test1_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 0x1);  /* Simple non-empty body */
        global_sum += (counter & 0x1);
    } while (--counter != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Test 2: while loop with post-decrement */
int test2_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += (counter & 0x3);
        global_sum += (counter & 0x3);
    }
    
    return local_sum;
}

/* Test 3: Nested loops - inner loop should show the pattern */
int test3_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        do {
            local_sum += (i * j) & 0xF;
            global_sum += (i * j) & 0xF;
        } while (--j != 0);  /* Inner do-while with pre-decrement */
    }
    
    return local_sum;
}

/* Test 4: Multiple decrementing loops in sequence */
int test4_multiple_loops(unsigned int n) {
    int local_sum = 0;
    unsigned int counter;
    
    /* First loop */
    counter = n;
    while (counter-- != 0) {
        local_sum += 1;
        global_sum += 1;
    }
    
    /* Second loop */
    counter = n / 2;
    do {
        local_sum += 2;
        global_sum += 2;
    } while (--counter != 0);
    
    return local_sum;
}

/* Test 5: Loop with unsigned int to ensure proper comparison */
int test5_unsigned_loop(unsigned int n) {
    unsigned int local_sum = 0;
    unsigned int counter = n;
    
    /* Force the pattern by using != 0 comparison */
    if (counter > 0) {
        do {
            local_sum += counter;
            global_sum += (int)(counter & 0xFF);
        } while (--counter != 0);
    }
    
    return (int)local_sum;
}

int main(int argc, char *argv[]) {
    int result = 0;
    unsigned int base_count;
    
    /* Use command line argument for loop bound, default to 100 */
    if (argc > 1) {
        base_count = (unsigned int)atoi(argv[1]);
        if (base_count == 0) base_count = 100;
    } else {
        base_count = 100;
    }
    
    printf("Testing with base_count = %u\n", base_count);
    
    /* Execute all test functions */
    result += test1_do_while_predec(base_count);
    result += test2_while_postdec((int)base_count);
    result += test3_nested_loops(base_count / 10, 10);
    result += test4_multiple_loops(base_count);
    result += test5_unsigned_loop(base_count);
    
    /* Also add global_sum to result for verification */
    result += global_sum;
    
    printf("Final result: %d\n", result);
    printf("Global sum: %d\n", global_sum);
    
    /* Return predictable result for verification */
    return (result > 0) ? 0 : 1;
}
