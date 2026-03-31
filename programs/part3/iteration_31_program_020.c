/* test-loop-doloop.c
 * Test program to trigger specific RTL pattern in loop-doloop.cc
 * Pattern needed: (set (cc) (compare (plus (reg) -1) (const_int 0)))
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variable to prevent dead code elimination */
volatile int global_sum = 0;

/* Test 1: do-while loop with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    if (n == 0) return 0;
    
    do {
        local_sum += 1;  /* Simple body to avoid optimization */
        global_sum += 1;
    } while (--n != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Test 2: while loop with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    
    while (n-- != 0) {  /* Post-decrement in condition */
        local_sum += 2;
        global_sum += 2;
    }
    
    return local_sum;
}

/* Test 3: Simple for loop that should compile to decrement pattern */
int test_for_loop(unsigned int n) {
    int local_sum = 0;
    unsigned int i;
    
    for (i = n; i != 0; i--) {  /* Decrement in update */
        local_sum += 3;
        global_sum += 3;
    }
    
    return local_sum;
}

/* Test 4: Nested loops - inner loop should show the pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        while (j-- != 0) {  /* Inner loop with post-decrement */
            local_sum += 4;
            global_sum += 4;
        }
    }
    
    return local_sum;
}

/* Test 5: Different integer type - unsigned short */
int test_short_loop(unsigned short n) {
    int local_sum = 0;
    
    do {
        local_sum += 5;
        global_sum += 5;
    } while (--n != 0);
    
    return local_sum;
}

/* Main function with command line argument for loop bounds */
int main(int argc, char *argv[]) {
    int base_iterations = 100;
    int result = 0;
    
    /* Use command line argument if provided, otherwise use default */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) {
            base_iterations = 100;
        }
        /* Cap to reasonable value */
        if (base_iterations > 10000) {
            base_iterations = 10000;
        }
    }
    
    printf("Testing with base_iterations = %d\n", base_iterations);
    
    /* Run all test variants */
    result += test_do_while_predec(base_iterations);
    result += test_while_postdec(base_iterations);
    result += test_for_loop(base_iterations);
    result += test_nested_loops(10, base_iterations / 10);
    result += test_short_loop((unsigned short)(base_iterations % 65535));
    
    /* Also test with global_sum to ensure side effects */
    printf("Result: %d, Global sum: %d\n", result, global_sum);
    
    /* Validate correctness */
    int expected = base_iterations * 1 +      /* test_do_while_predec */
                   base_iterations * 2 +      /* test_while_postdec */
                   base_iterations * 3 +      /* test_for_loop */
                   (10 * (base_iterations / 10)) * 4 +  /* test_nested_loops */
                   ((base_iterations % 65535)) * 5;     /* test_short_loop */
    
    if (result == expected) {
        printf("Test PASSED\n");
        return 0;
    } else {
        printf("Test FAILED: expected %d, got %d\n", expected, result);
        return 1;
    }
}
