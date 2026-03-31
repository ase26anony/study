/* test-loop-doloop.c
 * Test program to trigger specific RTL pattern in loop-doloop.cc
 * Pattern: SET with COMPARE of (PLUS reg -1) against const0_rtx
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent dead code elimination */
volatile int global_sum = 0;
volatile int global_check = 0;

/* Test 1: do-while loop with pre-decrement */
int test_dowhile_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 1);  /* Simple non-empty body */
        global_check ^= counter;     /* Side effect */
    } while (--counter != 0);        /* Pre-decrement in condition */
    
    return local_sum;
}

/* Test 2: while loop with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {         /* Post-decrement in condition */
        local_sum += (counter & 3);  /* Simple non-empty body */
        global_check += counter;     /* Side effect */
    }
    
    return local_sum;
}

/* Test 3: Nested loops - outer loop fixed, inner uses pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int total = 0;
    
    for (unsigned int i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner loop should generate the target pattern */
        do {
            total += (counter & 7);
            global_check |= counter;
        } while (--counter != 0);
    }
    
    return total;
}

/* Test 4: Multiple decrementing loops in same function */
int test_multiple_loops(int n) {
    int sum1 = 0, sum2 = 0;
    unsigned int c1 = n;
    int c2 = n;
    
    /* First loop: unsigned with pre-decrement */
    while (c1-- != 0) {
        sum1 += c1 * 2;
    }
    
    /* Second loop: signed with pre-decrement */
    do {
        sum2 += (c2 & 0xF);
        global_check ^= sum2;
    } while (--c2 != 0);
    
    return sum1 + sum2;
}

/* Test 5: Loop with simple arithmetic in body */
int test_arithmetic_loop(unsigned int n) {
    unsigned int counter = n;
    int result = 0;
    
    do {
        /* Simple arithmetic that doesn't obscure the decrement pattern */
        result = result * 3 + 1;
        global_sum += result & 0xFF;
    } while (--counter != 0);
    
    return result;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for loop bounds, but ensure minimum */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations < 10) base_iterations = 10;
        if (base_iterations > 10000) base_iterations = 10000;
    }
    
    printf("Testing with base_iterations = %d\n", base_iterations);
    
    /* Run all test functions to exercise different loop patterns */
    int result1 = test_dowhile_predec(base_iterations);
    int result2 = test_while_postdec(base_iterations);
    int result3 = test_nested_loops(5, base_iterations / 5);
    int result4 = test_multiple_loops(base_iterations);
    int result5 = test_arithmetic_loop(base_iterations);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2 + result3 + result4 + result5;
    final_result += global_sum + global_check;
    
    printf("Final result: %d\n", final_result);
    printf("Global sum: %d, Global check: %d\n", global_sum, global_check);
    
    return final_result != 0 ? 0 : 1;
}
