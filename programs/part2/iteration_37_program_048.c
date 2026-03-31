/* test_hw_doloop.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdlib.h>

/* Force architecture that supports hardware loops */
#ifdef __ARM_ARCH
__attribute__((target("arch=armv8-a")))
#elif __riscv
__attribute__((target("arch=rv64gc_zba")))
#endif
__attribute__((noinline, hot))
void test_loop_patterns(int n, int *results) {
    volatile int limit = n;  /* Prevent constant propagation */
    int i, j, k;
    
    /* Loop 1: Simple countable loop - will have its own basic blocks */
    int sum1 = 0;
    for (i = 0; i < limit; i++) {
        /* Simple body to remain hardware-loop eligible */
        sum1 += i * 2;
        results[0] = sum1;  /* Side effect */
    }
    
    /* Loop 2: Adjacent but disjoint loop - no block intersection with Loop 1 */
    int sum2 = 0;
    for (j = 0; j < limit; j++) {
        sum2 += j * 3;
        results[1] = sum2;
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    int sum3 = 0;
    int sum4 = 0;
    for (k = 0; k < limit; k++) {
        /* Loop 3 is entirely contained within Loop 4's blocks */
        for (int l = 0; l < 5; l++) {  /* Inner loop */
            sum3 += k * l;
            results[2] = sum3;
        }
        sum4 += k;
        results[3] = sum4;
    }
    
    /* Loop 5 and Loop 6: Partially overlapping loops using goto */
    /* This creates the bitmap_intersect_compl_p() condition */
    int sum5 = 0, sum6 = 0;
    int m = 0, p = 0;
    
    /* Loop 5 */
    do {
        sum5 += m * 7;
        results[4] = sum5;
        
        /* Conditional that may jump into Loop 6 */
        if (m % 3 == 0 && m < limit/2) {
            /* Jump to label inside Loop 6 */
            goto partial_overlap;
        }
        
        m++;
    } while (m < limit);
    
    /* Loop 6 - partially overlaps with Loop 5 via goto */
    p = 0;
    while (p < limit) {
        sum6 += p * 11;
        results[5] = sum6;
        
    partial_overlap:
        /* This label is inside Loop 6's body but reachable from Loop 5 */
        if (p % 2 == 0) {
            /* Another conditional to create more CFG complexity */
            sum6 += 1;
        }
        p++;
    }
    
    /* Loop 7: Another loop that shares exit blocks with Loop 8 */
    int sum7 = 0, sum8 = 0;
    int q = 0, r = 0;
    
    /* Loop 7 */
    for (q = 0; q < limit; q++) {
        sum7 += q;
        results[6] = sum7;
        
        /* Early exit that jumps to Loop 8's start */
        if (q == limit/2) {
            break;
        }
    }
    
    /* Loop 8: Shares some blocks via the break above */
    for (r = q; r < limit; r++) {
        sum8 += r * 2;
        results[7] = sum8;
    }
    
    /* Final computation to use all results */
    int total = sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7 + sum8;
    results[8] = total;
}

/* Helper to prevent optimization */
volatile int global_seed = 42;

int main() {
    int N = 100;
    int *results = (int*)malloc(10 * sizeof(int));
    
    /* Initialize with volatile to prevent constant folding */
    volatile int init = global_seed;
    for (int i = 0; i < 10; i++) {
        results[i] = init + i;
    }
    
    /* Call the function with all loop patterns */
    test_loop_patterns(N, results);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum ^= results[i];
        printf("results[%d] = %d\n", i, results[i]);
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(results);
    return checksum != 0 ? 0 : 1;
}
