/* test-doloop-pattern.c
 * Designed to trigger specific RTL pattern in loop-doloop.cc lines 136-150
 * Pattern: SET with COMPARE of (PLUS reg -1) against const0_rtx
 */

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

/* Variant 3: nested loops - inner loop with decrement pattern */
int test_nested_loops(unsigned int outer_iter, unsigned int inner_base) {
    int local_sum = 0;
    unsigned int outer = outer_iter;
    
    while (outer-- != 0) {
        unsigned int inner = inner_base + (outer & 7);  /* Vary inner count slightly */
        
        /* Inner loop with decrement pattern */
        do {
            local_sum += (inner & 1);
            global_sum += 3;
        } while (--inner != 0);
    }
    
    return local_sum;
}

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int local_sum = 0;
    unsigned int counter = (unsigned int)n;
    
    /* Force the pattern with unsigned counter */
    while (counter != 0) {
        local_sum += (int)counter;
        global_sum += 4;
        counter--;  /* Decrement in body, but condition checks counter != 0 */
    }
    
    return local_sum;
}

/* Variant 5: simple countdown loop */
int test_countdown(unsigned int n) {
    int local_sum = 0;
    
    for (unsigned int i = n; i != 0; --i) {  /* For loop with pre-decrement */
        local_sum += i;
        global_sum += 5;
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <loop_bound>\n", argv[0]);
        return 1;
    }
    
    int base_bound = atoi(argv[1]);
    if (base_bound <= 0) {
        base_bound = 100;  /* Default if invalid */
    }
    
    int total = 0;
    
    /* Run all test variants with different bounds derived from input */
    total += test_do_while_predec((unsigned int)base_bound);
    total += test_while_postdec(base_bound / 2);
    total += test_nested_loops(10, (unsigned int)base_bound / 10);
    total += test_mixed_types(base_bound / 3);
    total += test_countdown((unsigned int)base_bound);
    
    /* Print results to prevent dead code elimination */
    printf("Total: %d, Global: %d\n", total, global_sum);
    
    /* Return predictable value for verification */
    return (total > 0 && global_sum > 0) ? 0 : 1;
}
