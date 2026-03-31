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
    
    /* Additional arrays for complex access patterns */
    int src5_stride[N*2] __attribute__((aligned(32)));
    int src6_stride[N*2] __attribute__((aligned(32)));
    int mask_stride[N] __attribute__((aligned(32)));
    
    volatile int outer_bound = OUTER_ITER;
    uint32_t seed = 42;
    
    /* Initialize arrays with patterned data */
    for (int i = 0; i < N; i++) {
        src1_char[i] = (char)(simple_rand(&seed) % 256 - 128);
        src2_short[i] = (short)(simple_rand(&seed) % 65536 - 32768);
        src3_int[i] = (int)simple_rand(&seed);
        src4_long[i] = (long)simple_rand(&seed) * (i % 2 ? 1 : -1);
    }
    
    /* Initialize stride arrays */
    for (int i = 0; i < N*2; i++) {
        src5_stride[i] = i * 3;
        src6_stride[i] = i * 2 + 1;
    }
    
    /* Use argv to prevent compile-time constant propagation */
    int offset = argc > 1 ? atoi(argv[1]) : 0;
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Provide alignment hints to the compiler */
            char *aligned_char = __builtin_assume_aligned(src1_char, 32);
            short *aligned_short = __builtin_assume_aligned(src2_short, 32);
            
            /* GT_EXPR: a[i] > b[i] */
            mask_gt[i] = (aligned_char[i] > (char)(i + offset)) ? 1 : 0;
            
            /* GE_EXPR: a[i] >= b[i] */
            mask_ge[i] = (aligned_short[i] >= (short)(i * 2 + offset)) ? 1 : 0;
            
            /* LT_EXPR: a[i] < b[i] */
            mask_lt[i] = (src3_int[i] < (int)(i * 3 - offset)) ? 1 : 0;
            
            /* LE_EXPR: a[i] <= b[i] */
            mask_le[i] = (src4_long[i] <= (long)(i * 4 - offset)) ? 1 : 0;
            
            /* Additional comparisons with mixed types */
            int temp_gt = (src3_int[i] > src2_short[i]) ? 1 : 0;
            int temp_le = (src1_char[i] <= src3_int[i]) ? 1 : 0;
            
            /* Use results to prevent elimination */
            mask_gt[i] |= temp_gt;
            mask_le[i] |= temp_le;
        }
        
        /* Second inner loop with non-unit stride access */
        for (int i = 0; i < N; i++) {
            /* Compile-time constant stride of 2 */
            int *aligned_src5 = __builtin_assume_aligned(src5_stride, 32);
            int *aligned_src6 = __builtin_assume_aligned(src6_stride, 32);
            
            /* All four comparisons on stride-2 access pattern */
            int cmp1 = (aligned_src5[i*2] > aligned_src6[i*2]) ? 1 : 0;
            int cmp2 = (aligned_src5[i*2] >= aligned_src6[i*2 + 1]) ? 1 : 0;
            int cmp3 = (aligned_src5[i*2 + 1] < aligned_src6[i*2]) ? 1 : 0;
            int cmp4 = (aligned_src5[i*2 + 1] <= aligned_src6[i*2 + 1]) ? 1 : 0;
            
            /* Combine results */
            mask_stride[i] = cmp1 | (cmp2 << 1) | (cmp3 << 2) | (cmp4 << 3);
        }
        
        /* Conditional select operations using comparison results */
        for (int i = 0; i < N; i++) {
            /* Use comparisons in conditional select pattern */
            int val1 = (src3_int[i] > src2_short[i]) ? src3_int[i] : src2_short[i];
            int val2 = (src3_int[i] >= src2_short[i]) ? src3_int[i] + 1 : src2_short[i] - 1;
            int val3 = (src3_int[i] < src2_short[i]) ? src3_int[i] * 2 : src2_short[i] / 2;
            int val4 = (src3_int[i] <= src2_short[i]) ? src3_int[i] | 0xFF : src2_short[i] & 0xFF;
            
            /* Store results to prevent elimination */
            mask_gt[i] ^= val1;
            mask_ge[i] ^= val2;
            mask_lt[i] ^= val3;
            mask_le[i] ^= val4;
        }
    }
    
    /* Compute checksum to ensure side effects are observable */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += mask_gt[i];
        checksum += mask_ge[i];
        checksum += mask_lt[i];
        checksum += mask_le[i];
        checksum += mask_stride[i];
    }
    
    printf("Final checksum: %llu\n", checksum);
    return (int)(checksum % 256);
}
