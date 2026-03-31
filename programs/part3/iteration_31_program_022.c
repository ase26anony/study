/* test-doloop-pattern.c
 * Target compilation: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -fdump-rtl-all test-doloop-pattern.c -o test-doloop
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 0x1);  // Simple non-empty body
        global_sum++;
    } while (--counter != 0);
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {
        local_sum += (counter & 0x3);  // Different simple operation
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop has decrement pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        do {
            local_sum += i * j;
            global_sum += 3;
        } while (--j != 0);
    }
    
    return local_sum;
}

/* Variant 4: multiple decrementing loops in same function */
int test_multiple_loops(unsigned int n) {
    int local_sum = 0;
    unsigned int counter1 = n;
    unsigned int counter2 = n / 2;
    
    /* First loop: do-while */
    do {
        local_sum += 1;
        global_sum += 4;
    } while (--counter1 != 0);
    
    /* Second loop: while with post-decrement */
    while (counter2-- != 0) {
        local_sum += 2;
        global_sum += 5;
    }
    
    return local_sum;
}

/* Variant 5: unsigned char counter (different type) */
int test_char_counter(unsigned int n) {
    int local_sum = 0;
    unsigned char counter = (n > 255) ? 255 : n;
    
    do {
        local_sum += counter;
        global_sum += 6;
    } while (--counter != 0);
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for variability, but keep it reasonable */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations < 10) base_iterations = 10;
        if (base_iterations > 1000) base_iterations = 1000;
    }
    
    printf("Testing doloop patterns with base_iterations = %d\n", base_iterations);
    
    int result1 = test_do_while_predec(base_iterations);
    int result2 = test_while_postdec(base_iterations);
    int result3 = test_nested_loops(5, base_iterations / 5);
    int result4 = test_multiple_loops(base_iterations);
    int result5 = test_char_counter(base_iterations);
    
    int total = result1 + result2 + result3 + result4 + result5;
    
    printf("Results: %d, %d, %d, %d, %d\n", result1, result2, result3, result4, result5);
    printf("Total: %d, Global: %d\n", total, global_sum);
    
    /* Return predictable value for verification */
    return (total > 0 && global_sum > 0) ? 0 : 1;
}
