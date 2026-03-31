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
    volatile int use_arg = argc > 1;
    
    /* Source arrays with different integer types */
    char src1_char[N] __attribute__((aligned(32)));
    short src1_short[N] __attribute__((aligned(32)));
    int src1_int[N] __attribute__((aligned(32)));
    long src1_long[N] __attribute__((aligned(32)));
    
    char src2_char[N] __attribute__((aligned(32)));
    short src2_short[N] __attribute__((aligned(32)));
    int src2_int[N] __attribute__((aligned(32)));
    long src2_long[N] __attribute__((aligned(32)));
    
    /* Comparison result arrays */
    int mask_gt[N] __attribute__((aligned(32)));
    int mask_ge[N] __attribute__((aligned(32)));
    int mask_lt[N] __attribute__((aligned(32)));
    int mask_le[N] __attribute__((aligned(32)));
    
    /* Scalar values for mixed comparisons */
    volatile char scalar_char = 64;
    volatile short scalar_short = 128;
    volatile int scalar_int = 256;
    volatile long scalar_long = 512;
    
    uint32_t seed = 123456789;
    
    /* Initialize source arrays with patterned data */
    for (int i = 0; i < N; i++) {
        uint32_t r = simple_rand(&seed);
        src1_char[i] = (char)(r % 256);
        src1_short[i] = (short)(r % 65536);
        src1_int[i] = (int)r;
        src1_long[i] = (long)r * 3;
        
        r = simple_rand(&seed);
        src2_char[i] = (char)(r % 256);
        src2_short[i] = (short)(r % 65536);
        src2_int[i] = (int)r;
        src2_long[i] = (long)r * 2;
    }
    
    /* Provide alignment hints to the compiler */
    char * __restrict a_char = src1_char;
    char * __restrict b_char = src2_char;
    short * __restrict a_short = src1_short;
    short * __restrict b_short = src2_short;
    int * __restrict a_int = src1_int;
    int * __restrict b_int = src2_int;
    long * __restrict a_long = src1_long;
    long * __restrict b_long = src2_long;
    
    __builtin_assume_aligned(a_char, 32);
    __builtin_assume_aligned(b_char, 32);
    __builtin_assume_aligned(a_short, 32);
    __builtin_assume_aligned(b_short, 32);
    __builtin_assume_aligned(a_int, 32);
    __builtin_assume_aligned(b_int, 32);
    __builtin_assume_aligned(a_long, 32);
    __builtin_assume_aligned(b_long, 32);
    
    volatile int outer_bound = OUTER_ITER;
    long checksum = 0;
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Array vs Array comparisons - all four operators */
            mask_gt[i] = (a_int[i] > b_int[i]) ? 1 : 0;      /* GT_EXPR */
            mask_ge[i] = (a_int[i] >= b_int[i]) ? 1 : 0;     /* GE_EXPR */
            mask_lt[i] = (a_int[i] < b_int[i]) ? 1 : 0;      /* LT_EXPR */
            mask_le[i] = (a_int[i] <= b_int[i]) ? 1 : 0;     /* LE_EXPR */
            
            /* Array vs Scalar comparisons with different types */
            mask_gt[i] |= (a_char[i] > scalar_char) ? 2 : 0;
            mask_ge[i] |= (a_short[i] >= scalar_short) ? 2 : 0;
            mask_lt[i] |= (a_long[i] < scalar_long) ? 2 : 0;
            mask_le[i] |= (a_int[i] <= scalar_int) ? 2 : 0;
            
            /* Non-unit stride access pattern */
            if (i * 2 < N) {
                mask_gt[i] |= (a_int[i] > b_int[i * 2]) ? 4 : 0;
                mask_ge[i] |= (a_int[i] >= b_int[i * 2]) ? 4 : 0;
                mask_lt[i] |= (a_int[i] < b_int[i * 2]) ? 4 : 0;
                mask_le[i] |= (a_int[i] <= b_int[i * 2]) ? 4 : 0;
            }
            
            /* Conditional select operations using comparisons */
            int temp1 = (a_int[i] > b_int[i]) ? a_int[i] : b_int[i];
            int temp2 = (a_int[i] >= b_int[i]) ? a_int[i] + 1 : b_int[i] - 1;
            int temp3 = (a_int[i] < b_int[i]) ? a_int[i] * 2 : b_int[i] / 2;
            int temp4 = (a_int[i] <= b_int[i]) ? a_int[i] - 1 : b_int[i] + 1;
            
            mask_gt[i] += temp1;
            mask_ge[i] += temp2;
            mask_lt[i] += temp3;
            mask_le[i] += temp4;
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            checksum += mask_gt[i] + mask_ge[i] + mask_lt[i] + mask_le[i];
        }
        
        /* Modify source data slightly for next outer iteration */
        for (int i = 0; i < N; i++) {
            a_int[i] += outer;
            b_int[i] -= outer;
        }
    }
    
    /* Final observable side effect */
    printf("Checksum: %ld\n", checksum);
    
    /* Additional test with mixed-width comparisons */
    {
        int mixed_mask[N];
        for (int i = 0; i < N; i++) {
            /* Mixed type comparisons that may require type conversions */
            mixed_mask[i] = (a_char[i] > b_short[i]) ? 1 : 0;
            mixed_mask[i] |= (a_short[i] >= b_int[i]) ? 2 : 0;
            mixed_mask[i] |= (a_int[i] < b_long[i]) ? 4 : 0;
            mixed_mask[i] |= (a_long[i] <= b_char[i]) ? 8 : 0;
        }
        
        long mixed_checksum = 0;
        for (int i = 0; i < N; i++) {
            mixed_checksum += mixed_mask[i];
        }
        printf("Mixed checksum: %ld\n", mixed_checksum);
    }
    
    return 0;
}
