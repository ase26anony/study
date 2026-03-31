/* test-doloop-pattern.c
 * Test program to trigger specific RTL pattern in loop-doloop.cc
 * Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-doloop-pattern.c
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;

/* Test 1: do-while with pre-decrement */
int test1_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 1);  // Simple operation
        global_sum += local_sum;
    } while (--counter != 0);
    
    return local_sum;
}

/* Test 2: while loop with post-decrement */
int test2_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {
        local_sum += counter;
        global_sum ^= local_sum;
    }
    
    return local_sum;
}

/* Test 3: Nested loops - inner loop has decrement pattern */
int test3_nested_loops(unsigned int outer, unsigned int inner) {
    int total = 0;
    
    for (unsigned int i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner loop with decrement pattern */
        do {
            total += (i * counter);
            global_sum += total;
        } while (--counter != 0);
    }
    
    return total;
}

/* Test 4: Multiple decrement patterns in same function */
int test4_multiple_patterns(int a, int b) {
    int sum = 0;
    
    /* First pattern */
    unsigned int counter1 = a;
    while (counter1-- != 0) {
        sum += 1;
    }
    
    /* Second pattern */
    int counter2 = b;
    do {
        sum += 2;
        global_sum += sum;
    } while (--counter2 != 0);
    
    return sum;
}

/* Test 5: Complex but still recognizable pattern */
int test5_complex_decrement(unsigned int n) {
    int result = 0;
    unsigned int counter = n;
    
    /* Mix of operations but keep decrement pattern clear */
    do {
        result = (result * 31 + 17) & 0xFF;
        if (result > 100) {
            global_sum += result;
        }
    } while (--counter != 0);
    
    return result;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <loop_bound>\n", argv[0]);
        return 1;
    }
    
    int base_bound = atoi(argv[1]);
    if (base_bound <= 0) base_bound = 100;
    
    int total_result = 0;
    
    /* Run all tests with different bounds derived from input */
    total_result += test1_do_while_predec(base_bound);
    total_result += test2_while_postdec(base_bound / 2);
    total_result += test3_nested_loops(base_bound / 10, 5);
    total_result += test4_multiple_patterns(base_bound / 3, base_bound / 4);
    total_result += test5_complex_decrement(base_bound);
    
    /* Add global_sum to ensure side effects aren't optimized away */
    total_result += global_sum;
    
    printf("Result: %d\n", total_result);
    return 0;
}
