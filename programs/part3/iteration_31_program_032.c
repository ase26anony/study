/* Test program for doloop optimization pattern matching */
/* Specifically targets: SET with COMPARE of (PLUS reg -1) against const0_rtx */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent elimination */

/* Variant 1: do-while with pre-decrement */
int test_dowhile_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    if (counter == 0) return 0;
    
    do {
        local_sum += (counter & 0xFF);  /* Simple non-empty body */
        global_sum++;                   /* Side effect */
    } while (--counter != 0);           /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {            /* Post-decrement in condition */
        local_sum += (counter & 0x7F);  /* Simple non-empty body */
        global_sum += 2;                /* Side effect */
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop with decrement */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner loop with decrement pattern */
        do {
            local_sum += (i * j) & 0xFF;
            global_sum += 3;
            j = counter;  /* Use counter in computation */
        } while (--counter != 0);
    }
    
    return local_sum;
}

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int local_sum = 0;
    unsigned int counter;
    
    /* Ensure positive value for unsigned loop */
    counter = (n > 0) ? n : 10;
    
    while (counter != 0) {
        local_sum += counter;
        global_sum += 4;
        counter--;  /* Decrement in body, but condition checks counter != 0 */
    }
    
    return local_sum;
}

/* Variant 5: simple countdown loop */
int test_countdown(unsigned int n) {
    int local_sum = 0;
    
    /* Direct countdown from n to 1 */
    for (unsigned int i = n; i != 0; i--) {
        local_sum += i;
        global_sum += 5;
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    int result = 0;
    
    /* Use command line argument for variable but predictable iteration count */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) {
            base_iterations = 100;
        }
        /* Limit to reasonable values */
        if (base_iterations > 10000) {
            base_iterations = 10000;
        }
    }
    
    printf("Testing doloop patterns with base_iterations = %d\n", base_iterations);
    
    /* Execute all test variants */
    result += test_dowhile_predec(base_iterations);
    result += test_while_postdec(base_iterations);
    result += test_nested_loops(5, base_iterations / 5);
    result += test_mixed_types(base_iterations);
    result += test_countdown(base_iterations);
    
    /* Also test with different sizes */
    result += test_dowhile_predec(base_iterations / 2);
    result += test_while_postdec(base_iterations / 3);
    
    printf("Result: %d (global_sum: %d)\n", result, global_sum);
    
    /* Return consistent value for test verification */
    return (result > 0) ? 0 : 1;
}
