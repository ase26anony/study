/* test-doloop-pattern.c */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-doloop-pattern.c */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent complete optimization */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    if (n == 0) return 0;
    
    do {
        local_sum += (n & 0x1);  /* Simple non-empty body */
        global_sum++;
    } while (--n != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    
    while (n-- != 0) {  /* Post-decrement in condition */
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
        
        /* Inner do-while loop */
        do {
            total += (counter & 0x7);
            global_sum += 3;
        } while (--counter != 0);
    }
    
    return total;
}

/* Variant 4: mixed signed/unsigned */
int test_mixed_types(int n) {
    unsigned int u = (unsigned int)n;
    int sum = 0;
    
    /* Use unsigned counter */
    while (u-- != 0) {
        sum += (int)(u % 5);
        global_sum += 4;
    }
    
    return sum;
}

/* Variant 5: simple countdown loop */
int test_countdown(unsigned int n) {
    int sum = 0;
    
    /* Explicit countdown pattern */
    unsigned int count = n;
    do {
        sum += count;
        global_sum += 5;
    } while (count-- != 0);  /* Post-decrement, compare with 0 */
    
    return sum;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <loop_bound>\n", argv[0]);
        return 1;
    }
    
    int base = atoi(argv[1]);
    if (base <= 0) base = 100;
    
    int result = 0;
    
    /* Test different loop variants */
    result += test_do_while_predec((unsigned int)base);
    result += test_while_postdec(base);
    result += test_nested_loops(3, (unsigned int)(base / 3));
    result += test_mixed_types(base);
    result += test_countdown((unsigned int)(base % 50));
    
    printf("Result: %d, Global: %d\n", result, global_sum);
    
    /* Verify all loops executed */
    if (global_sum > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Loops were optimized away */
    }
}
