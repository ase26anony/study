/* Test program for doloop optimization with specific RTL pattern:
 * SET with COMPARE of (PLUS reg -1) against const0_rtx
 * Targeting architectures with condition code registers (PowerPC, SPARC)
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent elimination */

/* Variant 1: do-while with pre-decrement */
int test_dowhile_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 1);  /* Simple computation */
        global_sum++;
    } while (--counter != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += (counter & 3);
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop should generate pattern */
int test_nested(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner do-while loop */
        do {
            local_sum += i * j;
            global_sum += 3;
            j = counter;  /* Use counter in computation */
        } while (--counter != 0);  /* Should generate the pattern */
    }
    
    return local_sum;
}

/* Variant 4: Simple decrementing loop with unsigned */
int test_simple_decr(unsigned int n) {
    int local_sum = 0;
    
    /* Direct use of parameter in loop */
    while (n-- != 0) {  /* Post-decrement comparison with zero */
        local_sum += global_sum;
        global_sum ^= 0x55;  /* Simple modification */
    }
    
    return local_sum;
}

/* Variant 5: Mixed types to test different RTL generation */
int test_mixed_types(int start, unsigned int iterations) {
    int local_sum = 0;
    int counter = start + iterations;  /* Start from calculated value */
    
    do {
        local_sum += (counter & 0xF);
        global_sum -= 1;
    } while (--counter > 0);  /* Note: > 0 instead of != 0, but should still work */
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for variability, but ensure minimum iterations */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations < 10) base_iterations = 10;
        if (base_iterations > 10000) base_iterations = 10000;
    }
    
    printf("Testing doloop patterns with base_iterations = %d\n", base_iterations);
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Run all test variants */
    int result1 = test_dowhile_predec(base_iterations);
    int result2 = test_while_postdec(base_iterations);
    int result3 = test_nested(5, base_iterations / 5);
    int result4 = test_simple_decr(base_iterations);
    int result5 = test_mixed_types(10, base_iterations);
    
    /* Compute final result for validation */
    int final_result = result1 + result2 + result3 + result4 + result5 + global_sum;
    
    printf("Results: %d, %d, %d, %d, %d, global=%d\n", 
           result1, result2, result3, result4, result5, global_sum);
    printf("Final result: %d\n", final_result);
    
    /* Return predictable value for validation */
    return (final_result > 0) ? 0 : 1;
}
