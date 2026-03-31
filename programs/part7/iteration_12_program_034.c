#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define OUTER_ITER 10

/* Simple PRNG to generate non-constant data */
static uint32_t seed = 123456789;
static inline uint32_t rand_simple(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time propagation */
    volatile int use_arg = argc > 1 ? atoi(argv[1]) : 1;
    
    /* Declare source arrays with different integer types */
    char src1_char[N] __attribute__((aligned(64)));
    short src1_short[N] __attribute__((aligned(64)));
    int src1_int[N] __attribute__((aligned(64)));
    long src1_long[N] __attribute__((aligned(64)));
    
    char src2_char[N] __attribute__((aligned(64)));
    short src2_short[N] __attribute__((aligned(64)));
    int src2_int[N] __attribute__((aligned(64)));
    long src2_long[N] __attribute__((aligned(64)));
    
    /* Destination mask arrays for comparison results */
    int mask_gt[N] __attribute__((aligned(64)));
    int mask_ge[N] __attribute__((aligned(64)));
    int mask_lt[N] __attribute__((aligned(64)));
    int mask_le[N] __attribute__((aligned(64)));
    
    /* Initialize source arrays with patterned data */
    for (int i = 0; i < N; i++) {
        src1_char[i] = (char)((i * 3) % 256 - 128);
        src1_short[i] = (short)((i * 5) % 65536 - 32768);
        src1_int[i] = (int)(i * 7 - N/2);
        src1_long[i] = (long)(i * 11 - N);
        
        src2_char[i] = (char)((i * 13) % 256 - 128);
        src2_short[i] = (short)((i * 17) % 65536 - 32768);
        src2_int[i] = (int)(i * 19 - N/2);
        src2_long[i] = (long)(i * 23 - N);
    }
    
    /* Provide alignment hints to the compiler */
    char *p1c = __builtin_assume_aligned(src1_char, 64);
    char *p2c = __builtin_assume_aligned(src2_char, 64);
    short *p1s = __builtin_assume_aligned(src1_short, 64);
    short *p2s = __builtin_assume_aligned(src2_short, 64);
    int *p1i = __builtin_assume_aligned(src1_int, 64);
    int *p2i = __builtin_assume_aligned(src2_int, 64);
    long *p1l = __builtin_assume_aligned(src1_long, 64);
    long *p2l = __builtin_assume_aligned(src2_long, 64);
    int *pm_gt = __builtin_assume_aligned(mask_gt, 64);
    int *pm_ge = __builtin_assume_aligned(mask_ge, 64);
    int *pm_lt = __builtin_assume_aligned(mask_lt, 64);
    int *pm_le = __builtin_assume_aligned(mask_le, 64);
    
    volatile int outer_bound = OUTER_ITER;
    long total_checksum = 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Key inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Use different data types for different comparisons */
            /* GT_EXPR case: > comparison */
            pm_gt[i] = (p1i[i] > p2i[i]) ? 1 : 0;
            
            /* GE_EXPR case: >= comparison */
            pm_ge[i] = (p1l[i] >= p2l[i]) ? 1 : 0;
            
            /* LT_EXPR case: < comparison */
            pm_lt[i] = (p1s[i] < p2s[i]) ? 1 : 0;
            
            /* LE_EXPR case: <= comparison */
            pm_le[i] = (p1c[i] <= p2c[i]) ? 1 : 0;
            
            /* Additional comparisons with scalar */
            if (i % 4 == 0) {
                /* Mix in scalar comparisons */
                pm_gt[i] |= (p1i[i] > 0) ? 2 : 0;
                pm_ge[i] |= (p1l[i] >= 0L) ? 2 : 0;
                pm_lt[i] |= (p1s[i] < (short)use_arg) ? 2 : 0;
                pm_le[i] |= (p1c[i] <= (char)use_arg) ? 2 : 0;
            }
            
            /* Non-constant stride access pattern */
            if (i * 2 < N) {
                pm_gt[i] |= (p1i[i] > p2i[i * 2]) ? 4 : 0;
                pm_le[i] |= (p1c[i] <= p2c[i * 2]) ? 4 : 0;
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += pm_gt[i] + pm_ge[i] * 3 + pm_lt[i] * 5 + pm_le[i] * 7;
        }
        
        /* Modify source data slightly each outer iteration */
        for (int i = 0; i < N; i++) {
            p1i[i] += outer;
            p2i[i] -= outer % 3;
        }
    }
    
    /* Conditional select operations using comparison results */
    int select_results[N];
    for (int i = 0; i < N; i++) {
        /* Use comparisons in conditional select */
        select_results[i] = (p1i[i] > p2i[i]) ? p1i[i] : p2i[i];
        select_results[i] += (p1l[i] >= p2l[i]) ? p1i[i] : p2s[i];
        select_results[i] += (p1s[i] < p2s[i]) ? p1c[i] : p2c[i];
        select_results[i] += (p1c[i] <= p2c[i]) ? p1s[i] : p2i[i];
        total_checksum += select_results[i];
    }
    
    printf("Final checksum: %ld\n", total_checksum);
    return 0;
}
