/* test-loop-doloop.c */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-loop-doloop.c */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent complete optimization */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 1);  /* Simple non-empty body */
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

/* Variant 3: nested loops - inner loop with decrement */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner do-while loop */
        do {
            local_sum += i * j;
            global_sum += 3;
            j = counter;
        } while (--counter != 0);
    }
    
    return local_sum;
}

/* Variant 4: mixed unsigned/signed counters */
int test_mixed_counters(int n) {
    int local_sum = 0;
    unsigned int counter;
    
    /* Use absolute value to ensure positive counter */
    counter = (n < 0) ? -n : n;
    if (counter == 0) counter = 1;
    
    while (counter != 0) {
        local_sum += n;
        global_sum += 4;
        counter--;  /* Decrement in body, check in condition */
    }
    
    return local_sum;
}

/* Variant 5: simple countdown loop */
int test_countdown(unsigned int n) {
    int local_sum = 0;
    
    /* Force n to be at least 1 */
    unsigned int counter = (n == 0) ? 1 : n;
    
    for (; counter != 0; counter--) {
        local_sum += counter;
        global_sum += 5;
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_count;
    int result = 0;
    
    if (argc < 2) {
        base_count = 100;  /* Default if no argument */
    } else {
        base_count = atoi(argv[1]);
        if (base_count <= 0) base_count = 50;
    }
    
    printf("Testing with base count = %d\n", base_count);
    
    /* Run all test variants */
    result += test_do_while_predec(base_count);
    result += test_while_postdec(base_count);
    result += test_nested_loops(3, base_count / 3);
    result += test_mixed_counters(base_count);
    result += test_countdown(base_count);
    
    printf("Result: %d (global_sum: %d)\n", result, global_sum);
    
    /* Return non-zero if any test failed (simplified check) */
    return (result == 0) ? 1 : 0;
}
