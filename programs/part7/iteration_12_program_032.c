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
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride for complex patterns */
            int idx = i;
            
            /* GT_EXPR: > comparison */
            mask_gt_aligned[idx] = (src3_aligned[idx] > src3_aligned[(idx + 1) % N]) ? 1 : 0;
            
            /* GE_EXPR: >= comparison */
            mask_ge_aligned[idx] = (src2_aligned[idx] >= (short)(src1_aligned[idx] * 2)) ? 1 : 0;
            
            /* LT_EXPR: < comparison with swapped operands */
            mask_lt_aligned[idx] = ((long)(src1_aligned[idx]) < src4_aligned[idx]) ? 1 : 0;
            
            /* LE_EXPR: <= comparison with swapped operands */
            mask_le_aligned[idx] = (src3_aligned[idx] <= (int)(src4_aligned[idx] >> 2)) ? 1 : 0;
            
            /* Additional comparisons with scalar */
            mask_gt_aligned[idx] |= (src1_aligned[idx] > 0) ? 2 : 0;
            mask_ge_aligned[idx] |= (src2_aligned[idx] >= 100) ? 2 : 0;
            mask_lt_aligned[idx] |= (0 < src3_aligned[idx]) ? 2 : 0;
            mask_le_aligned[idx] |= (-100 <= src4_aligned[idx]) ? 2 : 0;
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i += 2) {  /* Non-unit stride */
            total_checksum += mask_gt_aligned[i] + mask_ge_aligned[i] 
                           + mask_lt_aligned[i] + mask_le_aligned[i];
        }
        
        /* Modify source data slightly each outer iteration */
        for (int i = 0; i < N; i++) {
            src3_aligned[i] += outer;
            src4_aligned[i] -= outer;
        }
    }
    
    /* Additional loop with mixed comparisons in conditional select */
    int select_results[N] __attribute__((aligned(32)));
    int *select_aligned = __builtin_assume_aligned(select_results, 32);
    
    for (int i = 0; i < N; i++) {
        /* Use comparisons in conditional select operations */
        select_aligned[i] = (src1_aligned[i] > src2_aligned[i]) ? 
                           src3_aligned[i] : src4_aligned[i];
        select_aligned[i] += (src3_aligned[i] >= src4_aligned[i]) ? 
                           src1_aligned[i] : src2_aligned[i];
        select_aligned[i] -= (src2_aligned[i] < src3_aligned[i]) ? 
                           src4_aligned[i] : src1_aligned[i];
        select_aligned[i] *= (src4_aligned[i] <= src1_aligned[i]) ? 
                           2 : 1;
    }
    
    /* Consume select results */
    for (int i = 0; i < N; i++) {
        total_checksum += select_aligned[i];
    }
    
    printf("Final checksum: %ld\n", total_checksum);
    return (int)(total_checksum % 256);
}
