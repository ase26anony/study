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
    
    /* Declare comparison result arrays */
    int mask_gt[N] __attribute__((aligned(32)));
    int mask_ge[N] __attribute__((aligned(32)));
    int mask_lt[N] __attribute__((aligned(32)));
    int mask_le[N] __attribute__((aligned(32)));
    
    /* Additional arrays for complex access patterns */
    int src_strided[N*2] __attribute__((aligned(32)));
    int mask_strided[N] __attribute__((aligned(32)));
    
    volatile int outer_bound = OUTER_ITER; /* Prevent outer loop unrolling */
    uint32_t seed = 42;
    
    /* Initialize arrays with patterned data */
    for (int i = 0; i < N; i++) {
        src1_char[i] = (char)(simple_rand(&seed) % 256 - 128);
        src2_short[i] = (short)(simple_rand(&seed) % 65536 - 32768);
        src3_int[i] = (int)simple_rand(&seed);
        src4_long[i] = (long)simple_rand(&seed) * (i % 2 ? 1 : -1);
        src_strided[i*2] = (int)simple_rand(&seed);
        src_strided[i*2 + 1] = (int)simple_rand(&seed);
    }
    
    /* Use argv to prevent compile-time constant propagation */
    int offset = (argc > 1) ? atoi(argv[1]) : 0;
    
    long total_checksum = 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Provide alignment hints */
            char *aligned_char = __builtin_assume_aligned(src1_char, 32);
            short *aligned_short = __builtin_assume_aligned(src2_short, 32);
            int *aligned_int = __builtin_assume_aligned(src3_int, 32);
            long *aligned_long = __builtin_assume_aligned(src4_long, 32);
            int *aligned_strided = __builtin_assume_aligned(src_strided, 32);
            
            /* GT_EXPR: > comparison */
            mask_gt[i] = (aligned_char[i] > (char)(i + offset)) ? 1 : 0;
            
            /* GE_EXPR: >= comparison with different type */
            mask_ge[i] = (aligned_short[i] >= (short)(i * 2 + offset)) ? 1 : 0;
            
            /* LT_EXPR: < comparison */
            mask_lt[i] = (aligned_int[i] < (int)(i - offset)) ? 1 : 0;
            
            /* LE_EXPR: <= comparison with different type */
            mask_le[i] = (aligned_long[i] <= (long)(i * 3 + offset)) ? 1 : 0;
            
            /* Additional comparison with non-unit stride */
            mask_strided[i] = (aligned_strided[i*2] > aligned_strided[i*2 + 1]) ? 1 : 0;
            
            /* Mixed-width comparisons in conditional select */
            int temp = (aligned_char[i] > aligned_short[i]) ? 
                      aligned_int[i] : aligned_long[i];
            mask_gt[i] ^= temp; /* Use result to prevent elimination */
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt[i] + mask_ge[i] * 2 + 
                            mask_lt[i] * 3 + mask_le[i] * 4 +
                            mask_strided[i] * 5;
        }
        
        /* Modify source data slightly each outer iteration */
        for (int i = 0; i < N; i++) {
            src1_char[i] += (i % 3);
            src2_short[i] -= (i % 5);
            src3_int[i] ^= (int)simple_rand(&seed);
        }
    }
    
    printf("Final checksum: %ld\n", total_checksum);
    return (total_checksum != 0) ? 0 : 1;
}
