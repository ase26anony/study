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
    /* Declare source arrays with different integer types */
    char src1_char[N] __attribute__((aligned(32)));
    short src2_short[N] __attribute__((aligned(32)));
    int src3_int[N] __attribute__((aligned(32)));
    long src4_long[N] __attribute__((aligned(32)));
    
    /* Declare comparison mask arrays */
    int mask_gt[N] __attribute__((aligned(32)));
    int mask_ge[N] __attribute__((aligned(32)));
    int mask_lt[N] __attribute__((aligned(32)));
    int mask_le[N] __attribute__((aligned(32)));
    
    /* Additional arrays for strided access */
    int src_strided[N*2] __attribute__((aligned(32)));
    int mask_strided[N] __attribute__((aligned(32)));
    
    /* Use argv to prevent compile-time propagation */
    uint32_t seed = (argc > 1) ? (uint32_t)atoi(argv[1]) : 42;
    
    /* Initialize source arrays with patterned data */
    for (int i = 0; i < N; i++) {
        uint32_t r = simple_rand(&seed);
        src1_char[i] = (char)(r & 0xFF);
        src2_short[i] = (short)(r & 0xFFFF);
        src3_int[i] = (int)r;
        src4_long[i] = (long)r;
        src_strided[i*2] = (int)(r % 100);
        src_strided[i*2 + 1] = (int)((r >> 8) % 100);
    }
    
    /* Provide alignment hints to the compiler */
    char *src1_aligned = __builtin_assume_aligned(src1_char, 32);
    short *src2_aligned = __builtin_assume_aligned(src2_short, 32);
    int *src3_aligned = __builtin_assume_aligned(src3_int, 32);
    long *src4_aligned = __builtin_assume_aligned(src4_long, 32);
    int *src_strided_aligned = __builtin_assume_aligned(src_strided, 32);
    int *mask_gt_aligned = __builtin_assume_aligned(mask_gt, 32);
    int *mask_ge_aligned = __builtin_assume_aligned(mask_ge, 32);
    int *mask_lt_aligned = __builtin_assume_aligned(mask_lt, 32);
    int *mask_le_aligned = __builtin_assume_aligned(mask_le, 32);
    int *mask_strided_aligned = __builtin_assume_aligned(mask_strided, 32);
    
    volatile int outer_bound = OUTER_ITER;  /* Prevent outer loop unrolling */
    int checksum = 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* GT_EXPR: char comparison */
            mask_gt_aligned[i] = (src1_aligned[i] > (char)(i & 0xFF)) ? 1 : 0;
            
            /* GE_EXPR: short comparison with scalar */
            mask_ge_aligned[i] = (src2_aligned[i] >= (short)1000) ? 1 : 0;
            
            /* LT_EXPR: int comparison */
            mask_lt_aligned[i] = (src3_aligned[i] < src3_aligned[(i + 1) % N]) ? 1 : 0;
            
            /* LE_EXPR: long comparison */
            mask_le_aligned[i] = (src4_aligned[i] <= src4_aligned[(i + N/2) % N]) ? 1 : 0;
            
            /* Additional comparison with strided access */
            mask_strided_aligned[i] = (src_strided_aligned[i*2] <= src_strided_aligned[i*2 + 1]) ? 1 : 0;
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            checksum += mask_gt_aligned[i] + mask_ge_aligned[i] + 
                       mask_lt_aligned[i] + mask_le_aligned[i] + 
                       mask_strided_aligned[i];
        }
        
        /* Modify source data slightly to prevent complete optimization */
        for (int i = 0; i < N; i++) {
            src1_aligned[i] += (char)(outer & 0x7F);
            src2_aligned[i] += (short)(outer * 10);
            src3_aligned[i] += outer;
            src4_aligned[i] += outer;
        }
    }
    
    /* Conditional select operations using comparison results */
    int result_select[N] __attribute__((aligned(32)));
    int *result_select_aligned = __builtin_assume_aligned(result_select, 32);
    
    for (int i = 0; i < N; i++) {
        /* Use comparisons in conditional select (?: operator) */
        result_select_aligned[i] = (src3_aligned[i] > src3_aligned[(i + 1) % N]) ? 
                                   src3_aligned[i] : src3_aligned[(i + 1) % N];
    }
    
    /* Final checksum computation */
    int final_checksum = checksum;
    for (int i = 0; i < N; i++) {
        final_checksum += result_select_aligned[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    return final_checksum != 0 ? 0 : 1;
}
