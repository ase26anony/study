/* Test program for doloop optimization with specific RTL pattern */
/* Compile with: -O2 -march=powerpc64 -fdump-rtl-doloop -fdump-rtl-all */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent elimination */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 0xFF);  /* Simple computation */
        global_sum += (counter & 0x1);
    } while (--counter != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += (counter & 0xFF);
        global_sum += (counter & 0x3);
    }
    
    return local_sum;
}

/* Variant 3: Simple decrementing loop */
int test_simple_decr(unsigned int n) {
    int local_sum = 0;
    unsigned int i = n;
    
    while (i) {  /* Will become (i-- != 0) after optimization */
        local_sum += i;
        global_sum += (i & 0x7);
        i--;
    }
    
    return local_sum;
}

/* Variant 4: Nested loops - inner loop should show the pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        do {
            local_sum += (i * j) & 0xFF;
            global_sum += (j & 0x1);
        } while (--j != 0);  /* Inner do-while with pre-decrement */
    }
    
    return local_sum;
}

/* Variant 5: Different integer type - unsigned short */
int test_short_loop(unsigned short n) {
    int local_sum = 0;
    unsigned short counter = n;
    
    do {
        local_sum += counter;
        global_sum += (counter & 0x1);
    } while (counter-- != 0);  /* Post-decrement with unsigned short */
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int total_sum = 0;
    unsigned int base_count;
    
    /* Use command line argument for loop bound, default to 100 */
    if (argc > 1) {
        base_count = (unsigned int)atoi(argv[1]);
        if (base_count == 0) base_count = 100;
    } else {
        base_count = 100;
    }
    
    printf("Testing doloop patterns with base count = %u\n", base_count);
    
    /* Test all variants */
    total_sum += test_do_while_predec(base_count);
    total_sum += test_while_postdec((int)base_count);
    total_sum += test_simple_decr(base_count);
    total_sum += test_nested_loops(10, base_count / 10);
    total_sum += test_short_loop((unsigned short)(base_count % 65535));
    
    printf("Total sum: %d\n", total_sum);
    printf("Global sum: %d\n", global_sum);
    
    /* Return non-zero if any test failed (simplified check) */
    return (total_sum == 0) ? 1 : 0;
}
