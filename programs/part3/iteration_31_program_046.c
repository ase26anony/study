/* test-doloop-pattern.c */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-doloop-pattern.c */
/* Or for SPARC: gcc -O3 -mcpu=ultrasparc -fdump-rtl-doloop -S test-doloop-pattern.c */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent complete optimization */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    if (n == 0) return 0;
    
    do {
        local_sum += (int)(--n);  /* Pre-decrement in body */
        global_sum++;
    } while (n != 0);  /* Compare against zero */
    
    return local_sum;
}

/* Variant 2: while with post-decrement in condition */
int test_while_postdec(int n) {
    int local_sum = 0;
    
    while (n--) {  /* Post-decrement compare against zero pattern */
        local_sum += n;
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: while with explicit decrement */
int test_while_explicit(unsigned int n) {
    int local_sum = 0;
    
    while (n != 0) {
        local_sum += (int)n;
        global_sum += 3;
        n--;  /* Decrement at end */
    }
    
    return local_sum;
}

/* Variant 4: Nested loops - inner loop should generate pattern */
int test_nested_loops(int outer, int inner) {
    int total = 0;
    
    for (int i = 0; i < outer; i++) {
        unsigned int counter = (unsigned int)inner;
        
        /* Inner do-while loop */
        do {
            total += counter;
            global_sum += 4;
        } while (--counter != 0);  /* Pre-decrement with explicit compare */
    }
    
    return total;
}

/* Variant 5: Mixed signed/unsigned */
int test_mixed_types(int n) {
    unsigned int u = (unsigned int)n;
    int sum = 0;
    
    if (u == 0) return 0;
    
    do {
        sum += (int)u;
        global_sum += 5;
    } while (--u != 0);
    
    return sum;
}

int main(int argc, char *argv[]) {
    int base_count = 100;
    
    /* Use command line argument for variability, but keep it reasonable */
    if (argc > 1) {
        base_count = atoi(argv[1]);
        if (base_count <= 0) base_count = 100;
        if (base_count > 10000) base_count = 10000; /* Prevent excessive runtime */
    }
    
    int result = 0;
    
    /* Execute all variants */
    result += test_do_while_predec((unsigned int)base_count);
    result += test_while_postdec(base_count);
    result += test_while_explicit((unsigned int)base_count);
    result += test_nested_loops(3, base_count / 3);
    result += test_mixed_types(base_count);
    
    /* Print results to ensure loops execute */
    printf("Result: %d\n", result);
    printf("Global sum: %d\n", global_sum);
    
    /* Expected values for verification */
    int expected_result = 0;
    int n = base_count;
    
    /* Calculate expected results */
    expected_result += (n * (n - 1)) / 2;  /* test_do_while_predec */
    expected_result += (n * (n - 1)) / 2;  /* test_while_postdec */
    expected_result += (n * (n + 1)) / 2 - n;  /* test_while_explicit */
    expected_result += 3 * ((n/3) * ((n/3) + 1)) / 2;  /* test_nested_loops */
    expected_result += (n * (n + 1)) / 2 - n;  /* test_mixed_types */
    
    printf("Expected result: %d\n", expected_result);
    
    return (result == expected_result) ? 0 : 1;
}
