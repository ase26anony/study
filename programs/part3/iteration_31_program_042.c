/* test-loop-doloop.c - Test program to trigger specific RTL patterns in loop-doloop.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global variable to prevent dead code elimination */
volatile int global_sum = 0;

/* Test 1: Basic do-while loop with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 0xFF);  /* Simple non-empty body */
        global_sum += (counter & 0x1);
    } while (--counter != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Test 2: While loop with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += (counter & 0xFF);
        global_sum ^= (counter & 0x1);
    }
    
    return local_sum;
}

/* Test 3: Nested loops - inner loop uses decrement pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int total = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        do {
            total += (i * j);
            global_sum += (j & 0x1);
        } while (--j != 0);  /* Inner do-while with pre-decrement */
    }
    
    return total;
}

/* Test 4: Multiple decrementing loops in same function */
int test_multiple_loops(int n) {
    int sum1 = 0, sum2 = 0;
    int counter1 = n;
    unsigned int counter2 = n * 2;
    
    /* First loop: do-while with pre-decrement */
    do {
        sum1 += counter1;
        global_sum += 1;
    } while (--counter1 != 0);
    
    /* Second loop: while with post-decrement */
    while (counter2-- != 0) {
        sum2 += counter2;
        global_sum -= 1;
    }
    
    return sum1 + sum2;
}

/* Test 5: Loop with unsigned counter - different type */
unsigned int test_unsigned_loop(unsigned int n) {
    unsigned int hash = 0;
    unsigned int counter = n;
    
    do {
        hash = (hash << 3) ^ counter;  /* Simple hash computation */
        global_sum += (hash & 0x1);
    } while (--counter != 0);
    
    return hash;
}

/* Test 6: Loop that's not trivially unrollable */
int test_variable_bound(int start) {
    int sum = 0;
    int counter = start;
    
    /* Use a variable bound that's not compile-time constant */
    if (counter <= 0) counter = 10;  /* Ensure positive */
    
    do {
        sum += (counter * counter);
        global_sum = (global_sum + 1) & 0xFF;
    } while (--counter != 0);
    
    return sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    int result = 0;
    
    /* Use command line argument for loop bounds, but with a reasonable default */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) {
            base_iterations = 50;
        }
        if (base_iterations > 10000) {
            base_iterations = 10000;  /* Prevent excessive runtime */
        }
    }
    
    printf("Testing loop patterns with base_iterations = %d\n", base_iterations);
    
    /* Run all test functions to exercise different patterns */
    result += test_do_while_predec(base_iterations);
    result += test_while_postdec(base_iterations);
    result += test_nested_loops(5, base_iterations / 5);
    result += test_multiple_loops(base_iterations / 2);
    result += test_unsigned_loop(base_iterations);
    result += test_variable_bound(base_iterations);
    
    /* Also use the global sum to prevent optimization */
    result += global_sum;
    
    printf("Final result: %d (global_sum: %d)\n", result, global_sum);
    
    /* Return non-zero if something went wrong (though we can't easily verify RTL here) */
    return (result == 0) ? 1 : 0;
}
