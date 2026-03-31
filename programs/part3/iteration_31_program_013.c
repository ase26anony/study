/* test-doloop-pattern.c
 * Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-doloop-pattern.c
 * Or for coverage: gcc -O2 -march=powerpc64 -fdump-rtl-all test-doloop-pattern.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variable to prevent dead code elimination */
volatile int global_sum = 0;

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    if (n == 0) return 0;
    
    do {
        local_sum += (int)n;
        global_sum += 1;
    } while (--n != 0);
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    
    while (n--) {
        local_sum += n;  /* n is already decremented here */
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop uses decrement pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int total = 0;
    
    for (unsigned int i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner loop with decrement pattern */
        do {
            total += (counter * i);
            global_sum += 3;
        } while (--counter != 0);
    }
    
    return total;
}

/* Variant 4: mixed signed/unsigned types */
int test_mixed_types(int n) {
    unsigned int u = (unsigned int)n;
    int sum = 0;
    
    /* Use unsigned counter with decrement */
    while (u-- != 0) {
        sum += (int)u;
        global_sum += 4;
    }
    
    return sum;
}

/* Variant 5: simple countdown loop */
int test_countdown(int iterations) {
    int sum = 0;
    int count = iterations;
    
    while (count != 0) {
        sum += count;
        global_sum += 5;
        count--;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <loop_bound>\n", argv[0]);
        return 1;
    }
    
    int base = atoi(argv[1]);
    if (base <= 0) base = 10;
    
    int total = 0;
    
    /* Test all variants with different bounds derived from input */
    total += test_do_while_predec((unsigned int)base);
    total += test_while_postdec(base);
    total += test_nested_loops(base % 5 + 1, base % 3 + 2);
    total += test_mixed_types(base);
    total += test_countdown(base);
    
    printf("Total: %d, Global sum: %d\n", total, global_sum);
    
    /* Return value based on results for verification */
    return (total > 0 && global_sum > 0) ? 0 : 1;
}
