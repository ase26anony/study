/* test-doloop-pattern.c */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-doloop-pattern.c */
/* Or for SPARC: gcc -O3 -mcpu=ultrasparc -fdump-rtl-doloop -S test-doloop-pattern.c */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent complete optimization */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    if (counter == 0) return 0;
    
    do {
        local_sum += (counter & 0x1);  /* Simple non-empty body */
        global_sum++;
    } while (--counter != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += (counter & 0x3);
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: Nested loops - inner loop with decrement pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner do-while loop */
        if (counter != 0) {
            do {
                local_sum += i * j;
                global_sum += 3;
                j = counter;  /* Use counter in computation */
            } while (--counter != 0);
        }
    }
    
    return local_sum;
}

/* Variant 4: Mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int local_sum = 0;
    unsigned int counter;
    
    /* Convert to unsigned for clean decrement pattern */
    counter = (unsigned int)(n > 0 ? n : 1);
    
    while (counter != 0) {
        local_sum += (int)counter;
        global_sum += 4;
        counter--;  /* Separate decrement, but condition checks against 0 */
    }
    
    return local_sum;
}

/* Variant 5: Simple countdown loop */
int test_countdown(unsigned int n) {
    int local_sum = 0;
    
    /* Explicit countdown pattern */
    for (unsigned int i = n; i != 0; i--) {
        local_sum += i;
        global_sum += 5;
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_count;
    int result = 0;
    
    /* Use command line argument to set loop bounds, preventing compile-time elimination */
    if (argc > 1) {
        base_count = atoi(argv[1]);
        if (base_count <= 0) base_count = 100;
    } else {
        base_count = 100;
    }
    
    /* Limit size to avoid overflow in simple tests */
    if (base_count > 10000) base_count = 10000;
    
    printf("Testing with base_count = %d\n", base_count);
    
    /* Execute all variants */
    result += test_do_while_predec((unsigned int)base_count);
    result += test_while_postdec(base_count);
    result += test_nested_loops(5, (unsigned int)(base_count / 5));
    result += test_mixed_types(base_count);
    result += test_countdown((unsigned int)base_count);
    
    printf("Result: %d (global_sum: %d)\n", result, global_sum);
    
    /* Return deterministic result for validation */
    return (result > 0) ? 0 : 1;
}
