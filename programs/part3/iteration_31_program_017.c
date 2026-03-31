/* test-loop-doloop.c
 * Test program to trigger specific RTL pattern in loop-doloop.cc
 * Pattern: SET with COMPARE of (PLUS reg -1) against const0_rtx
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent dead code elimination */

/* Test 1: do-while loop with pre-decrement */
int test1_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 0x1);  /* Simple non-empty body */
        global_sum++;
    } while (--counter != 0);
    
    return local_sum;
}

/* Test 2: while loop with post-decrement */
int test2_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {
        local_sum += (counter & 0x3);  /* Different simple operation */
        global_sum += 2;
    }
    
    return local_sum;
}

/* Test 3: Nested loops - outer loop fixed, inner uses pattern */
int test3_nested_loops(unsigned int outer_iter, unsigned int inner_base) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer_iter; i++) {
        j = inner_base;
        /* Inner loop should generate the target pattern */
        do {
            local_sum += i * j;
            global_sum += 3;
        } while (--j != 0);
    }
    
    return local_sum;
}

/* Test 4: Multiple decrementing loops in sequence */
int test4_multiple_loops(unsigned int n1, unsigned int n2, unsigned int n3) {
    int local_sum = 0;
    unsigned int counter;
    
    /* First loop */
    counter = n1;
    while (counter-- != 0) {
        local_sum += 1;
        global_sum += 4;
    }
    
    /* Second loop */
    counter = n2;
    do {
        local_sum += 2;
        global_sum += 5;
    } while (--counter != 0);
    
    /* Third loop - mixed types */
    {
        int signed_counter = (int)n3;
        while (signed_counter-- != 0) {
            local_sum += 3;
            global_sum += 6;
        }
    }
    
    return local_sum;
}

/* Test 5: Loop with compile-time known bound but using variable */
int test5_known_bound(unsigned int multiplier) {
    int local_sum = 0;
    unsigned int counter = 10 * multiplier;  /* Known factor, variable multiplier */
    
    /* This should still generate the pattern */
    do {
        local_sum += multiplier;
        global_sum += 7;
    } while (--counter != 0);
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int result = 0;
    unsigned int base_iterations;
    
    /* Use command line argument for variable loop bounds */
    if (argc > 1) {
        base_iterations = (unsigned int)atoi(argv[1]);
        if (base_iterations == 0) {
            base_iterations = 5;  /* Default */
        }
        if (base_iterations > 1000) {
            base_iterations = 1000;  /* Cap to avoid long execution */
        }
    } else {
        base_iterations = 5;
    }
    
    printf("Testing with base_iterations = %u\n", base_iterations);
    
    /* Execute all test functions */
    result += test1_do_while_predec(base_iterations);
    result += test2_while_postdec((int)base_iterations);
    result += test3_nested_loops(3, base_iterations);
    result += test4_multiple_loops(base_iterations, base_iterations/2, base_iterations*2);
    result += test5_known_bound(base_iterations % 10 + 1);
    
    printf("Result: %d\n", result);
    printf("Global sum: %d\n", global_sum);
    
    /* Return 0 only if global_sum matches expected pattern */
    if (global_sum > 0) {
        return 0;
    } else {
        return 1;  /* Indicate failure if loops were optimized away */
    }
}
