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
    short src2_short[N] __attribute__((aligned(32)));
    int src3_int[N] __attribute__((aligned(32)));
    long src4_long[N] __attribute__((aligned(32)));
    
    /* Scalar values for mixed comparisons */
    volatile char scalar_char = 64;
    volatile short scalar_short = 128;
    volatile int scalar_int = 256;
    volatile long scalar_long = 512;
    
    /* Destination mask arrays for comparison results */
    int mask_gt[N] __attribute__((aligned(32)));
    int mask_ge[N] __attribute__((aligned(32)));
    int mask_lt[N] __attribute__((aligned(32)));
    int mask_le[N] __attribute__((aligned(32)));
    
    /* Additional arrays for conditional select operations */
    int select_result[N] __attribute__((aligned(32)));
    int alt_values[N] __attribute__((aligned(32)));
    
    uint32_t seed = 42;
    
    /* Initialize arrays with patterned data */
    for (int i = 0; i < N; i++) {
        uint32_t r = simple_rand(&seed);
        src1_char[i] = (char)(r % 256);
        src2_short[i] = (short)(r % 65536);
        src3_int[i] = (int)r;
        src4_long[i] = (long)r * 2;
        alt_values[i] = (int)(r % 1000);
    }
    
    /* Provide alignment hints to the compiler */
    char *aligned_char = __builtin_assume_aligned(src1_char, 32);
    short *aligned_short = __builtin_assume_aligned(src2_short, 32);
    int *aligned_int = __builtin_assume_aligned(src3_int, 32);
    long *aligned_long = __builtin_assume_aligned(src4_long, 32);
    
    volatile int outer_bound = OUTER_ITER;
    long total_checksum = 0;
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Key inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* GT_EXPR: greater than comparison */
            mask_gt[i] = (aligned_char[i] > scalar_char) ? 1 : 0;
            
            /* GE_EXPR: greater than or equal comparison */
            mask_ge[i] = (aligned_short[i] >= scalar_short) ? 1 : 0;
            
            /* LT_EXPR: less than comparison */
            mask_lt[i] = (aligned_int[i] < scalar_int) ? 1 : 0;
            
            /* LE_EXPR: less than or equal comparison */
            mask_le[i] = (aligned_long[i] <= scalar_long) ? 1 : 0;
            
            /* Use comparison results in conditional select */
            select_result[i] = (aligned_char[i] > scalar_char) ? 
                              alt_values[i] : aligned_int[i % (N/2)];
        }
        
        /* Second inner loop with array-to-array comparisons and non-unit stride */
        for (int i = 0; i < N/2; i++) {
            /* Access with compile-time constant stride of 2 */
            int idx = i * 2;
            
            /* All four comparisons between arrays */
            mask_gt[idx] |= (aligned_char[idx] > aligned_char[(idx + 1) % N]) ? 1 : 0;
            mask_ge[idx] |= (aligned_short[idx] >= aligned_short[(idx + 2) % N]) ? 1 : 0;
            mask_lt[idx] |= (aligned_int[idx] < aligned_int[(idx + 3) % N]) ? 1 : 0;
            mask_le[idx] |= (aligned_long[idx] <= aligned_long[(idx + 4) % N]) ? 1 : 0;
        }
        
        /* Third loop with mixed-width comparisons */
        for (int i = 0; i < N; i++) {
            /* Compare different integer types */
            mask_gt[i] |= ((int)aligned_char[i] > aligned_int[i]) ? 2 : 0;
            mask_ge[i] |= ((long)aligned_short[i] >= aligned_long[i]) ? 2 : 0;
            mask_lt[i] |= (aligned_int[i] < (int)aligned_char[i]) ? 2 : 0;
            mask_le[i] |= (aligned_long[i] <= (long)aligned_short[i]) ? 2 : 0;
        }
        
        /* Compute checksum to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt[i] + mask_ge[i] + mask_lt[i] + mask_le[i];
            total_checksum += select_result[i];
        }
        
        /* Modify source data slightly for next outer iteration */
        for (int i = 0; i < N; i++) {
            aligned_char[i] += (i % 3);
            aligned_short[i] += (i % 5);
            aligned_int[i] += (i % 7);
            aligned_long[i] += (i % 11);
        }
    }
    
    printf("Final checksum: %ld\n", total_checksum);
    
    /* Use the result to prevent optimization */
    if (use_arg) {
        return (int)(total_checksum % 1000);
    }
    return 0;
}
