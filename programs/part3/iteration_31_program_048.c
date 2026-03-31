/* test-loop-doloop.c
 * Test program to trigger specific RTL pattern in loop-doloop.cc
 * Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-loop-doloop.c
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    if (n == 0) return 0;
    
    do {
        local_sum += (n & 0x1);  // Simple operation
        global_sum++;
    } while (--n != 0);  /* This should generate: (compare (plus reg -1) 0) */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    
    while (n-- != 0) {  /* This should also generate the pattern */
        local_sum += (n & 0x3);
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop with decrement */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int total = 0;
    
    for (unsigned int i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner loop with decrement pattern */
        do {
            total += (counter & 0x7);
            global_sum += 3;
        } while (--counter != 0);
    }
    
    return total;
}

/* Variant 4: mixed signed/unsigned */
int test_mixed_types(int iterations) {
    unsigned int counter = (unsigned int)iterations;
    int result = 0;
    
    if (counter == 0) return 0;
    
    do {
        result += (int)(counter % 5);
        global_sum += 4;
    } while (--counter != 0);
    
    return result;
}

/* Variant 5: while loop with explicit decrement in body */
int test_explicit_decrement(unsigned int n) {
    int sum = 0;
    
    while (n != 0) {
        sum += n;
        global_sum += 5;
        n--;  /* Decrement in body, compare at top */
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <loop_count>\n", argv[0]);
        return 1;
    }
    
    int base_count = atoi(argv[1]);
    if (base_count <= 0) {
        base_count = 100;  /* Default if invalid */
    }
    
    int total_result = 0;
    
    /* Test all variants with different loop counts */
    total_result += test_do_while_predec(base_count);
    total_result += test_while_postdec(base_count / 2);
    total_result += test_nested_loops(5, base_count / 5);
    total_result += test_mixed_types(base_count / 3);
    total_result += test_explicit_decrement(base_count / 4);
    
    printf("Result: %d (Global sum: %d)\n", total_result, global_sum);
    
    /* Return deterministic result for verification */
    return (total_result > 0) ? 0 : 1;
}
