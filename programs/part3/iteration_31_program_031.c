/* test-doloop.c - Test for doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>

/* Global variable to prevent dead code elimination */
volatile int global_sum = 0;

/* Test 1: do-while loop with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 1);  /* Simple operation */
        global_sum++;
    } while (--counter != 0);
    
    return local_sum;
}

/* Test 2: while loop with post-decrement */
int test_while_postdec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    while (counter-- != 0) {
        local_sum += (counter & 3);  /* Different simple operation */
        global_sum += 2;
    }
    
    return local_sum;
}

/* Test 3: Nested loops - inner loop uses decrement pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        while (j-- != 0) {
            local_sum += i * j;
            global_sum += 3;
        }
    }
    
    return local_sum;
}

/* Test 4: Mixed signed/unsigned counters */
int test_signed_counter(int n) {
    int local_sum = 0;
    int counter = n;
    
    /* Use do-while to ensure at least one iteration */
    if (counter > 0) {
        do {
            local_sum += counter;
            global_sum += 4;
        } while (--counter != 0);
    }
    
    return local_sum;
}

/* Test 5: Simple countdown loop */
int test_countdown(unsigned int n) {
    int local_sum = 0;
    
    /* Direct countdown in condition */
    while (n-- != 0) {
        local_sum += n;
        global_sum += 5;
    }
    
    return local_sum;
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
    
    printf("Testing doloop patterns with base_count = %u\n", base_count);
    
    /* Run all test variants */
    result += test_do_while_predec(base_count);
    result += test_while_postdec(base_count);
    result += test_nested_loops(base_count / 10, 10);
    result += test_signed_counter((int)base_count);
    result += test_countdown(base_count);
    
    printf("Result: %d\n", result);
    printf("Global sum: %d\n", global_sum);
    
    /* Return 0 for success, non-zero if something went wrong */
    return (result == 0 && global_sum == 0) ? 1 : 0;
}
