#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define OUTER_ITER 10

/* Simple PRNG to generate non-constant data */
static inline uint32_t simple_rand(uint32_t *seed) {
    *seed = *seed * 1103515245 + 12345;
    return *seed;
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time propagation */
    volatile int use_arg = argc > 1 ? atoi(argv[1]) : 1;
    
    /* Declare source arrays with different integer types */
    char src1_char[N] __attribute__((aligned(32)));
    short src2_short[N] __attribute__((aligned(32)));
    int src3_int[N] __attribute__((aligned(32)));
    long src4_long[N] __attribute__((aligned(32)));
    
    /* Declare comparison result arrays */
    int mask_gt[N] __attribute__((aligned(32)));
    int mask_ge[N] __attribute__((aligned(32)));
    int mask_lt[N] __attribute__((aligned(32)));
    int mask_le[N] __attribute__((aligned(32)));
    
    /* Initialize with patterned data */
    uint32_t seed = 42;
    for (int i = 0; i < N; i++) {
        src1_char[i] = (char)(simple_rand(&seed) % 256 - 128);
        src2_short[i] = (short)(simple_rand(&seed) % 65536 - 32768);
        src3_int[i] = (int)simple_rand(&seed);
        src4_long[i] = (long)simple_rand(&seed) * use_arg;
    }
    
    /* Provide alignment hints */
    char *src1_aligned = __builtin_assume_aligned(src1_char, 32);
    short *src2_aligned = __builtin_assume_aligned(src2_short, 32);
    int *src3_aligned = __builtin_assume_aligned(src3_int, 32);
    long *src4_aligned = __builtin_assume_aligned(src4_long, 32);
    int *mask_gt_aligned = __builtin_assume_aligned(mask_gt, 32);
    int *mask_ge_aligned = __builtin_assume_aligned(mask_ge, 32);
    int *mask_lt_aligned = __builtin_assume_aligned(mask_lt, 32);
    int *mask_le_aligned = __builtin_assume_aligned(mask_le, 32);
    
    volatile int outer_bound = OUTER_ITER;
    long total_checksum = 0;
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride to create pattern */
            int idx = i;
            
            /* GT_EXPR: char comparison */
            mask_gt_aligned[idx] = (src1_aligned[idx] > (char)(i % 256 - 128)) ? 1 : 0;
            
            /* GE_EXPR: short comparison with scalar */
            mask_ge_aligned[idx] = (src2_aligned[idx] >= (short)(i % 1000)) ? 1 : 0;
            
            /* LT_EXPR: int comparison with array element */
            int cmp_val = (i % 2 == 0) ? src3_aligned[(i + 1) % N] : src3_aligned[(i - 1 + N) % N];
            mask_lt_aligned[idx] = (src3_aligned[idx] < cmp_val) ? 1 : 0;
            
            /* LE_EXPR: long comparison with mixed operation */
            long threshold = (long)(src4_aligned[idx] / 2) + outer * 100;
            mask_le_aligned[idx] = (src4_aligned[idx] <= threshold) ? 1 : 0;
            
            /* Additional comparisons with different stride patterns */
            if (i * 2 < N) {
                /* GT_EXPR with stride 2 */
                mask_gt_aligned[i * 2] |= (src1_aligned[i * 2] > src1_aligned[i]) ? 2 : 0;
                
                /* GE_EXPR with stride 2 */
                mask_ge_aligned[i * 2] |= (src2_aligned[i * 2] >= src2_aligned[i]) ? 2 : 0;
                
                /* LT_EXPR with stride 2 */
                mask_lt_aligned[i * 2] |= (src3_aligned[i * 2] < src3_aligned[i]) ? 2 : 0;
                
                /* LE_EXPR with stride 2 */
                mask_le_aligned[i * 2] |= (src4_aligned[i * 2] <= src4_aligned[i]) ? 2 : 0;
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt_aligned[i] + mask_ge_aligned[i] 
                            + mask_lt_aligned[i] + mask_le_aligned[i];
        }
        
        /* Modify source data slightly for next outer iteration */
        for (int i = 0; i < N; i++) {
            src1_aligned[i] += (char)(outer % 4);
            src2_aligned[i] += (short)(outer % 8);
            src3_aligned[i] += outer;
            src4_aligned[i] += outer * 1000L;
        }
    }
    
    /* Conditional select operations using comparison results */
    int select_results[N];
    for (int i = 0; i < N; i++) {
        /* Use GT comparison for conditional select */
        select_results[i] = (src1_aligned[i] > (char)(i % 128)) ? 
                           src3_aligned[i] : -src3_aligned[i];
        
        /* Use GE comparison for conditional select */
        select_results[i] += (src2_aligned[i] >= (short)(i % 256)) ? 
                            src3_aligned[i] : src3_aligned[i] / 2;
        
        /* Use LT comparison for conditional select */
        select_results[i] -= (src3_aligned[i] < src3_aligned[(i + 1) % N]) ? 
                            i : i * 2;
        
        /* Use LE comparison for conditional select */
        select_results[i] *= (src4_aligned[i] <= src4_aligned[(i + 2) % N]) ? 
                            1 : 2;
    }
    
    /* Final checksum computation */
    long final_checksum = total_checksum;
    for (int i = 0; i < N; i++) {
        final_checksum += select_results[i];
        final_checksum += mask_gt_aligned[i] * mask_ge_aligned[i];
        final_checksum -= mask_lt_aligned[i] * mask_le_aligned[i];
    }
    
    printf("Final checksum: %ld\n", final_checksum);
    return (final_checksum > 0) ? 0 : 1;
}
