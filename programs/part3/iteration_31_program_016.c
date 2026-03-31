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
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner do-while loop */
        do {
            local_sum += (i * j) & 0xF;
            global_sum += 3;
            j = counter;  /* Use counter in computation */
        } while (--counter != 0);
    }
    
    return local_sum;
}

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int local_sum = 0;
    unsigned int counter;
    
    /* Use unsigned for the decrement pattern */
    if (n > 0) {
        counter = (unsigned int)n;
        do {
            local_sum += n;
            global_sum += 4;
        } while (--counter != 0);
    }
    
    return local_sum;
}

/* Variant 5: while loop with explicit decrement */
int test_explicit_decrement(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    while (counter != 0) {
        local_sum += counter;
        global_sum += 5;
        counter--;  /* Decrement at end of loop */
        /* The optimizer should combine decrement and compare */
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    int result = 0;
    
    /* Use command line argument for variable but bounded iteration count */
    if (argc > 1) {
        base_iterations = atoi(argv[1]) % 1000;  /* Bound to reasonable value */
        if (base_iterations <= 0) base_iterations = 100;
    }
    
    printf("Testing with base_iterations = %d\n", base_iterations);
    
    /* Test all variants */
    result += test_do_while_predec(base_iterations);
    result += test_while_postdec(base_iterations);
    result += test_nested_loops(5, base_iterations / 5);
    result += test_mixed_types(base_iterations);
    result += test_explicit_decrement(base_iterations);
    
    printf("Result: %d (global_sum: %d)\n", result, global_sum);
    
    /* Return deterministic result for verification */
    return (result > 0) ? 0 : 1;
}
