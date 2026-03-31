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
    volatile int use_arg = (argc > 1);
    
    /* Source arrays with different integer types */
    char src1_char[N] __attribute__((aligned(64)));
    short src2_short[N] __attribute__((aligned(64)));
    int src3_int[N] __attribute__((aligned(64)));
    long src4_long[N] __attribute__((aligned(64)));
    
    /* Comparison result arrays */
    int mask_gt[N] __attribute__((aligned(64)));
    int mask_ge[N] __attribute__((aligned(64)));
    int mask_lt[N] __attribute__((aligned(64)));
    int mask_le[N] __attribute__((aligned(64)));
    
    /* Scalar values for mixed comparisons */
    volatile char scalar_char = 50;
    volatile short scalar_short = 100;
    volatile int scalar_int = 200;
    volatile long scalar_long = 300;
    
    /* Initialize source arrays with patterned data */
    uint32_t seed = 123456789;
    for (int i = 0; i < N; i++) {
        src1_char[i] = (char)(simple_rand(&seed) % 256 - 128);
        src2_short[i] = (short)(simple_rand(&seed) % 65536 - 32768);
        src3_int[i] = (int)(simple_rand(&seed) % 1000 - 500);
        src4_long[i] = (long)(simple_rand(&seed) % 2000 - 1000);
    }
    
    /* Provide alignment hints to the compiler */
    char *aligned_char = __builtin_assume_aligned(src1_char, 64);
    short *aligned_short = __builtin_assume_aligned(src2_short, 64);
    int *aligned_int = __builtin_assume_aligned(src3_int, 64);
    long *aligned_long = __builtin_assume_aligned(src4_long, 64);
    int *aligned_mask_gt = __builtin_assume_aligned(mask_gt, 64);
    int *aligned_mask_ge = __builtin_assume_aligned(mask_ge, 64);
    int *aligned_mask_lt = __builtin_assume_aligned(mask_lt, 64);
    int *aligned_mask_le = __builtin_assume_aligned(mask_le, 64);
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    volatile int outer_limit = OUTER_ITER;
    long total_checksum = 0;
    
    for (int outer = 0; outer < outer_limit; outer++) {
        /* Key inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride (i*2) for complex patterns */
            int idx = (i * 2) % N;
            
            /* GT_EXPR: greater than comparison */
            aligned_mask_gt[idx] = (aligned_char[idx] > scalar_char) ? 1 : 0;
            
            /* GE_EXPR: greater than or equal comparison */
            aligned_mask_ge[idx] = (aligned_short[idx] >= scalar_short) ? 1 : 0;
            
            /* LT_EXPR: less than comparison */
            aligned_mask_lt[idx] = (aligned_int[idx] < scalar_int) ? 1 : 0;
            
            /* LE_EXPR: less than or equal comparison */
            aligned_mask_le[idx] = (aligned_long[idx] <= scalar_long) ? 1 : 0;
            
            /* Additional array-to-array comparisons to ensure all cases are hit */
            if (i % 4 == 0) {
                /* GT_EXPR between arrays */
                aligned_mask_gt[idx] |= (aligned_char[idx] > aligned_char[(idx + 1) % N]) ? 2 : 0;
                
                /* GE_EXPR between arrays */
                aligned_mask_ge[idx] |= (aligned_short[idx] >= aligned_short[(idx + 1) % N]) ? 2 : 0;
                
                /* LT_EXPR between arrays */
                aligned_mask_lt[idx] |= (aligned_int[idx] < aligned_int[(idx + 1) % N]) ? 2 : 0;
                
                /* LE_EXPR between arrays */
                aligned_mask_le[idx] |= (aligned_long[idx] <= aligned_long[(idx + 1) % N]) ? 2 : 0;
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += aligned_mask_gt[i] + aligned_mask_ge[i] 
                            + aligned_mask_lt[i] + aligned_mask_le[i];
        }
        
        /* Modify source data slightly to prevent complete optimization */
        if (use_arg) {
            for (int i = 0; i < N; i += 8) {
                aligned_char[i] += (char)(outer % 4);
                aligned_short[i] += (short)(outer % 8);
                aligned_int[i] += (int)(outer % 16);
                aligned_long[i] += (long)(outer % 32);
            }
        }
    }
    
    /* Nested loop with different access pattern */
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < N; i++) {
            /* Mixed type comparisons in nested loop */
            int idx = (i + j) % N;
            
            /* Conditional select based on comparisons */
            int temp_gt = (aligned_char[idx] > (scalar_char + j)) ? 
                         aligned_int[idx] : aligned_int[(idx + 1) % N];
            
            int temp_ge = (aligned_short[idx] >= (scalar_short + j)) ? 
                         aligned_int[idx] : aligned_int[(idx + 1) % N];
            
            int temp_lt = (aligned_int[idx] < (scalar_int + j)) ? 
                         aligned_int[idx] : aligned_int[(idx + 1) % N];
            
            int temp_le = (aligned_long[idx] <= (scalar_long + j)) ? 
                         aligned_int[idx] : aligned_int[(idx + 1) % N];
            
            aligned_mask_gt[idx] ^= temp_gt;
            aligned_mask_ge[idx] ^= temp_ge;
            aligned_mask_lt[idx] ^= temp_lt;
            aligned_mask_le[idx] ^= temp_le;
        }
    }
    
    /* Final checksum computation */
    long final_checksum = 0;
    for (int i = 0; i < N; i++) {
        final_checksum += aligned_mask_gt[i] * 3 + aligned_mask_ge[i] * 5
                        + aligned_mask_lt[i] * 7 + aligned_mask_le[i] * 11;
    }
    
    final_checksum += total_checksum;
    
    printf("Final checksum: %ld\n", final_checksum);
    return (final_checksum != 0) ? 0 : 1;
}
