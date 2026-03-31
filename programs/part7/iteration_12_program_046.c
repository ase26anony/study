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
    /* Use argc to prevent compile-time constant propagation */
    volatile int use_arg = argc > 1 ? atoi(argv[1]) : 1;
    
    /* Source arrays with different integer types */
    char src1_char[N] __attribute__((aligned(32)));
    short src2_short[N] __attribute__((aligned(32)));
    int src3_int[N] __attribute__((aligned(32)));
    long src4_long[N] __attribute__((aligned(32)));
    
    /* Destination mask arrays for comparison results */
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
    
    /* Outer loop to trigger outer-loop vectorization */
    volatile int outer_bound = OUTER_ITER;
    long total_checksum = 0;
    
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Key inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride to create complex patterns */
            int idx = i;
            
            /* GT_EXPR: char comparison */
            mask_gt_aligned[idx] = (src1_aligned[idx] > (char)(use_arg + i % 128)) ? 1 : 0;
            
            /* GE_EXPR: short comparison with scalar */
            mask_ge_aligned[idx] = (src2_aligned[idx] >= (short)(use_arg * 10)) ? 1 : 0;
            
            /* LT_EXPR: int comparison with array element */
            mask_lt_aligned[idx] = (src3_aligned[idx] < src3_aligned[(idx + 1) % N]) ? 1 : 0;
            
            /* LE_EXPR: long comparison */
            mask_le_aligned[idx] = (src4_aligned[idx] <= src4_aligned[(idx + N/2) % N]) ? 1 : 0;
        }
        
        /* Additional loop with mixed comparisons on same data */
        for (int i = 0; i < N/2; i++) {
            /* Access with stride 2 for non-unit stride pattern */
            int idx = i * 2;
            
            /* All four comparisons on same data type (int) */
            int temp_gt = (src3_aligned[idx] > src3_aligned[idx + 1]) ? 1 : 0;
            int temp_ge = (src3_aligned[idx] >= src3_aligned[idx + 1]) ? 1 : 0;
            int temp_lt = (src3_aligned[idx] < src3_aligned[idx + 1]) ? 1 : 0;
            int temp_le = (src3_aligned[idx] <= src3_aligned[idx + 1]) ? 1 : 0;
            
            /* Use results in conditional select operations */
            mask_gt_aligned[idx] = temp_gt ? mask_gt_aligned[idx] : temp_ge;
            mask_ge_aligned[idx] = temp_lt ? mask_ge_aligned[idx] : temp_le;
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt_aligned[i] + mask_ge_aligned[i] 
                           + mask_lt_aligned[i] + mask_le_aligned[i];
        }
    }
    
    /* Final computation to ensure side effects are observable */
    printf("Final checksum: %ld\n", total_checksum);
    
    return (int)(total_checksum % 256);
}
