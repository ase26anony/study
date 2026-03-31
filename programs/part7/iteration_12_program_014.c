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
    short src1_short[N] __attribute__((aligned(32)));
    int src1_int[N] __attribute__((aligned(32)));
    long src1_long[N] __attribute__((aligned(32)));
    
    char src2_char[N] __attribute__((aligned(32)));
    short src2_short[N] __attribute__((aligned(32)));
    int src2_int[N] __attribute__((aligned(32)));
    long src2_long[N] __attribute__((aligned(32)));
    
    /* Destination mask arrays for comparison results */
    int mask_gt[N] __attribute__((aligned(32)));
    int mask_ge[N] __attribute__((aligned(32)));
    int mask_lt[N] __attribute__((aligned(32)));
    int mask_le[N] __attribute__((aligned(32)));
    
    /* Additional arrays for strided access */
    int src1_strided[N*2] __attribute__((aligned(32)));
    int src2_strided[N*2] __attribute__((aligned(32)));
    int mask_strided[N] __attribute__((aligned(32)));
    
    /* Scalar comparison values */
    volatile int scalar_comp = 50;
    volatile long scalar_comp_long = 100;
    
    /* Initialize with pattern to avoid compile-time propagation */
    uint32_t seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    for (int i = 0; i < N; i++) {
        src1_char[i] = simple_rand(&seed) % 256 - 128;
        src2_char[i] = simple_rand(&seed) % 256 - 128;
        
        src1_short[i] = simple_rand(&seed) % 65536 - 32768;
        src2_short[i] = simple_rand(&seed) % 65536 - 32768;
        
        src1_int[i] = simple_rand(&seed);
        src2_int[i] = simple_rand(&seed);
        
        src1_long[i] = (long)simple_rand(&seed) * simple_rand(&seed);
        src2_long[i] = (long)simple_rand(&seed) * simple_rand(&seed);
        
        /* Strided arrays */
        src1_strided[i*2] = simple_rand(&seed) % 1000;
        src2_strided[i*2] = simple_rand(&seed) % 1000;
    }
    
    /* Use alignment hints */
    char *p1 = (char*)__builtin_assume_aligned(src1_char, 32);
    char *p2 = (char*)__builtin_assume_aligned(src2_char, 32);
    int *p_strided = (int*)__builtin_assume_aligned(src1_strided, 32);
    
    volatile int outer_bound = OUTER_ITER;
    long long total_checksum = 0;
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* GT_EXPR - triggers case GT_EXPR */
            mask_gt[i] = (src1_int[i] > src2_int[i]) ? 1 : 0;
            
            /* GE_EXPR - triggers case GE_EXPR */
            mask_ge[i] = (src1_int[i] >= src2_int[i]) ? 1 : 0;
            
            /* LT_EXPR - triggers case LT_EXPR */
            mask_lt[i] = (src1_int[i] < src2_int[i]) ? 1 : 0;
            
            /* LE_EXPR - triggers case LE_EXPR */
            mask_le[i] = (src1_int[i] <= src2_int[i]) ? 1 : 0;
        }
        
        /* Second loop with different data types */
        for (int i = 0; i < N; i++) {
            /* Mixed type comparisons */
            int gt_char = (src1_char[i] > src2_char[i]) ? 1 : 0;
            int ge_short = (src1_short[i] >= src2_short[i]) ? 1 : 0;
            int lt_long = (src1_long[i] < src2_long[i]) ? 1 : 0;
            int le_mixed = (src1_int[i] <= scalar_comp) ? 1 : 0;
            
            /* Use results to prevent elimination */
            mask_gt[i] |= gt_char;
            mask_ge[i] |= ge_short;
            mask_lt[i] |= lt_long;
            mask_le[i] |= le_mixed;
        }
        
        /* Third loop with strided access pattern */
        for (int i = 0; i < N; i++) {
            /* Compile-time constant stride of 2 */
            mask_strided[i] = (src1_strided[i*2] > src2_strided[i*2]) ? 1 : 0;
            mask_strided[i] |= (src1_strided[i*2] >= scalar_comp) ? 2 : 0;
            mask_strided[i] |= (src1_strided[i*2] < src2_strided[i*2]) ? 4 : 0;
            mask_strided[i] |= (src1_strided[i*2] <= scalar_comp_long) ? 8 : 0;
        }
        
        /* Conditional select operations using comparisons */
        for (int i = 0; i < N; i++) {
            /* Use comparisons in conditional selects */
            int val1 = (src1_int[i] > src2_int[i]) ? src1_int[i] : src2_int[i];
            int val2 = (src1_int[i] >= scalar_comp) ? src1_int[i] : scalar_comp;
            int val3 = (src1_int[i] < src2_int[i]) ? src1_int[i] : src2_int[i];
            int val4 = (src1_int[i] <= scalar_comp_long) ? src1_int[i] : scalar_comp;
            
            /* Accumulate checksum to prevent dead code elimination */
            total_checksum += val1 + val2 + val3 + val4;
        }
    }
    
    /* Final checksum computation using all mask arrays */
    for (int i = 0; i < N; i++) {
        total_checksum += mask_gt[i] + mask_ge[i] + mask_lt[i] + mask_le[i];
        total_checksum += mask_strided[i];
    }
    
    printf("Final checksum: %lld\n", total_checksum);
    return 0;
}
