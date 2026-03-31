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
    
    /* Source arrays with different integer types */
    char src1_char[N] __attribute__((aligned(32)));
    short src2_short[N] __attribute__((aligned(32)));
    int src3_int[N] __attribute__((aligned(32)));
    long src4_long[N] __attribute__((aligned(32)));
    
    /* Destination mask arrays */
    int mask_gt[N] __attribute__((aligned(32)));
    int mask_ge[N] __attribute__((aligned(32)));
    int mask_lt[N] __attribute__((aligned(32)));
    int mask_le[N] __attribute__((aligned(32)));
    
    /* Initialize with patterned data */
    uint32_t seed = 42;
    for (int i = 0; i < N; i++) {
        src1_char[i] = (char)(simple_rand(&seed) % 256 - 128);
        src2_short[i] = (short)(simple_rand(&seed) % 65536 - 32768);
        src3_int[i] = (int)(simple_rand(&seed) % 1000 - 500);
        src4_long[i] = (long)(simple_rand(&seed) % 2000 - 1000);
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
    long long total_checksum = 0;
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride (every other element) for some arrays */
            int idx = (i * 2) % N;
            
            /* GT_EXPR: char comparison */
            mask_gt_aligned[i] = (src1_aligned[i] > (char)(use_arg * 10)) ? 1 : 0;
            
            /* GE_EXPR: short comparison with scalar */
            mask_ge_aligned[i] = (src2_aligned[idx] >= (short)(use_arg * 20)) ? 1 : 0;
            
            /* LT_EXPR: int comparison */
            mask_lt_aligned[i] = (src3_aligned[i] < (int)(use_arg * 30)) ? 1 : 0;
            
            /* LE_EXPR: long comparison with stride access */
            mask_le_aligned[i] = (src4_aligned[idx] <= (long)(use_arg * 40)) ? 1 : 0;
            
            /* Additional mixed-type comparisons to stress conversion logic */
            if (i % 4 == 0) {
                /* Cross-type comparisons */
                mask_gt_aligned[i] |= (src1_aligned[i] > (char)src2_aligned[i]) ? 2 : 0;
                mask_ge_aligned[i] |= (src2_aligned[i] >= (short)src3_aligned[i]) ? 2 : 0;
                mask_lt_aligned[i] |= (src3_aligned[i] < (int)src4_aligned[i]) ? 2 : 0;
                mask_le_aligned[i] |= (src4_aligned[i] <= (long)src1_aligned[i]) ? 2 : 0;
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt_aligned[i] + mask_ge_aligned[i] 
                            + mask_lt_aligned[i] + mask_le_aligned[i];
        }
        
        /* Modify source data slightly for next outer iteration */
        for (int i = 0; i < N; i += 4) {
            src1_aligned[i] += 1;
            src2_aligned[i] -= 1;
            src3_aligned[i] += 2;
            src4_aligned[i] -= 2;
        }
    }
    
    printf("Final checksum: %lld\n", total_checksum);
    return (total_checksum > 0) ? 0 : 1;
}
