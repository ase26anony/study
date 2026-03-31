/* test-loop-doloop.c
 * Designed to trigger specific RTL pattern in loop-doloop.cc lines 136-150
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
        global_sum++;
    } while (--counter != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Test 2: while loop with post-decrement */
int test2_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += (counter & 0x3);
        global_sum += 2;
    }
    
    return local_sum;
}

/* Test 3: Nested loops - inner loop uses decrement pattern */
int test3_nested_loops(unsigned int outer_iter, unsigned int inner_iter) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer_iter; i++) {
        unsigned int counter = inner_iter;
        
        /* Inner loop with decrement pattern */
        do {
            local_sum += i * j;
            global_sum += 3;
        } while (--counter != 0);
    }
    
    return local_sum;
}

/* Test 4: Multiple decrement patterns in same function */
int test4_multiple_patterns(int n) {
    int sum1 = 0, sum2 = 0;
    unsigned int counter1 = n;
    int counter2 = n;
    
    /* First pattern: unsigned do-while */
    do {
        sum1 += (counter1 % 5);
        global_sum += 4;
    } while (--counter1 != 0);
    
    /* Second pattern: int while loop */
    while (counter2-- != 0) {
        sum2 += (counter2 % 7);
        global_sum += 5;
    }
    
    return sum1 + sum2;
}

/* Test 5: Complex condition but still decrement pattern */
int test5_complex_body(int n) {
    int local_sum = 0;
    int counter = n;
    int temp[10] = {0};
    
    while (counter-- != 0) {
        /* More complex body but still simple enough for doloop */
        temp[counter % 10] += counter;
        local_sum += temp[counter % 10];
        global_sum += 6;
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for loop bound, but ensure it's reasonable */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) base_iterations = 100;
        if (base_iterations > 10000) base_iterations = 10000; /* Prevent excessive runtime */
    }
    
    printf("Testing loop patterns with base_iterations = %d\n", base_iterations);
    
    int result1 = test1_do_while_predec(base_iterations);
    int result2 = test2_while_postdec(base_iterations);
    int result3 = test3_nested_loops(5, base_iterations / 5);
    int result4 = test4_multiple_patterns(base_iterations);
    int result5 = test5_complex_body(base_iterations);
    
    int total_result = result1 + result2 + result3 + result4 + result5;
    
    printf("Results: %d, %d, %d, %d, %d\n", result1, result2, result3, result4, result5);
    printf("Total result: %d\n", total_result);
    printf("Global sum: %d\n", global_sum);
    
    /* Return consistent value for test verification */
    return (total_result > 0 && global_sum > 0) ? 0 : 1;
}
