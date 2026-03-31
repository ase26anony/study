/* Test program for doloop optimization pattern matching */
/* Compile with: -O2 -march=powerpc64 -fdump-rtl-doloop */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent elimination */

/* Variant 1: do-while with pre-decrement */
int test_dowhile_predec(unsigned int n) {
    int local_sum = 0;
    if (n == 0) return 0;
    
    do {
        local_sum += (n & 1);  /* Simple operation */
        global_sum++;
    } while (--n != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    
    while (n-- != 0) {  /* Post-decrement in condition */
        local_sum += (n & 3);
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop should match pattern */
int test_nested(unsigned int outer, unsigned int inner) {
    int total = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        /* Inner loop with decrement pattern */
        while (j-- != 0) {
            total += i * j;
            global_sum += 3;
        }
    }
    
    return total;
}

/* Variant 4: mixed signed/unsigned */
int test_mixed_types(int iterations) {
    unsigned int counter;
    int sum = 0;
    
    /* Force counter to be unsigned but compare against 0 */
    counter = (unsigned int)iterations;
    
    do {
        sum += (counter % 5);
        global_sum += 4;
    } while (--counter != 0);
    
    return sum;
}

/* Variant 5: Simple countdown loop */
int test_countdown(unsigned int n) {
    int result = 0;
    unsigned int count = n;
    
    /* Explicit countdown pattern */
    while (count != 0) {
        result += count;
        global_sum += 5;
        count--;  /* Decrement in body, but may still generate pattern */
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    int total_result = 0;
    
    /* Use command line argument for variable but predictable iteration count */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) base_iterations = 100;
    }
    
    printf("Testing doloop patterns with base iterations = %d\n", base_iterations);
    
    /* Test all variants */
    total_result += test_dowhile_predec(base_iterations);
    total_result += test_while_postdec(base_iterations / 2);
    total_result += test_nested(10, base_iterations / 10);
    total_result += test_mixed_types(base_iterations / 3);
    total_result += test_countdown(base_iterations);
    
    printf("Total result: %d\n", total_result);
    printf("Global sum: %d\n", global_sum);
    
    /* Return consistent value for test verification */
    return (total_result > 0) ? 0 : 1;
}
