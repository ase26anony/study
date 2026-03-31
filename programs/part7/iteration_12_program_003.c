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
    /* Use argc to prevent compile-time optimization */
    volatile int use_arg = argc > 1 ? atoi(argv[1]) : 1;
    
    /* Source arrays with different integer types */
    char src1_char[N] __attribute__((aligned(32)));
    short src2_short[N] __attribute__((aligned(32)));
    int src3_int[N] __attribute__((aligned(32)));
    long src4_long[N] __attribute__((aligned(32)));
    
    /* Destination arrays for comparison results */
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
    
    /* Provide alignment hints to the compiler */
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
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride (i*2) for complex patterns */
            int idx = (i * 2) % N;
            
            /* GT_EXPR: char comparison */
            mask_gt_aligned[idx] = (src1_aligned[idx] > (char)(use_arg + i % 128)) ? 1 : 0;
            
            /* GE_EXPR: short comparison */
            mask_ge_aligned[idx] = (src2_aligned[idx] >= (short)(use_arg * 2 - i % 256)) ? 1 : 0;
            
            /* LT_EXPR: int comparison */
            mask_lt_aligned[idx] = (src3_aligned[idx] < (int)(use_arg * 3 + i % 512)) ? 1 : 0;
            
            /* LE_EXPR: long comparison */
            mask_le_aligned[idx] = (src4_aligned[idx] <= (long)(use_arg * 4 - i % 1024)) ? 1 : 0;
            
            /* Additional comparisons with mixed types */
            if (i % 4 == 0) {
                /* Cross-type comparisons to stress conversion logic */
                mask_gt_aligned[idx] |= (src1_aligned[idx] > src2_aligned[idx]) ? 2 : 0;
                mask_ge_aligned[idx] |= (src2_aligned[idx] >= src3_aligned[idx]) ? 2 : 0;
                mask_lt_aligned[idx] |= (src3_aligned[idx] < src4_aligned[idx]) ? 2 : 0;
                mask_le_aligned[idx] |= (src4_aligned[idx] <= src1_aligned[idx]) ? 2 : 0;
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt[i] + mask_ge[i] * 3 + mask_lt[i] * 5 + mask_le[i] * 7;
        }
        
        /* Modify source data slightly for next outer iteration */
        for (int i = 0; i < N; i += 4) {
            src1_char[i] += 1;
            src2_short[i] -= 1;
            src3_int[i] += 2;
            src4_long[i] -= 2;
        }
    }
    
    /* Conditional select operations using comparison results */
    int select_results[N];
    for (int i = 0; i < N; i++) {
        /* Use comparisons in conditional select */
        select_results[i] = (src1_char[i] > src2_short[i]) ? 
                           src3_int[i] : src4_long[i];
        
        /* Chain comparisons for complex patterns */
        if (src1_char[i] >= src2_short[i] && src3_int[i] < src4_long[i]) {
            select_results[i] += mask_gt[i];
        }
    }
    
    /* Final checksum computation */
    long final_checksum = total_checksum;
    for (int i = 0; i < N; i++) {
        final_checksum += select_results[i];
    }
    
    printf("Final checksum: %ld\n", final_checksum);
    return (final_checksum > 0) ? 0 : 1;
}
