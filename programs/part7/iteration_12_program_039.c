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
    
    /* Provide alignment hints */
    char *src1_aligned = __builtin_assume_aligned(src1_char, 32);
    short *src2_aligned = __builtin_assume_aligned(src2_short, 32);
    int *src3_aligned = __builtin_assume_aligned(src3_int, 32);
    long *src4_aligned = __builtin_assume_aligned(src4_long, 32);
    int *mask_gt_aligned = __builtin_assume_aligned(mask_gt, 32);
    int *mask_ge_aligned = __builtin_assume_aligned(mask_ge, 32);
    int *mask_lt_aligned = __builtin_assume_aligned(mask_lt, 32);
    int *mask_le_aligned = __builtin_assume_aligned(mask_le, 32);
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    volatile int outer_bound = OUTER_ITER;
    long total_checksum = 0;
    
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride to create pattern */
            int idx = i;
            
            /* All four comparison operators on different data types */
            mask_gt_aligned[idx] = (src3_aligned[idx] > src2_aligned[idx]) ? 1 : 0;  /* GT_EXPR */
            mask_ge_aligned[idx] = (src4_aligned[idx] >= src3_aligned[idx]) ? 1 : 0; /* GE_EXPR */
            mask_lt_aligned[idx] = (src1_aligned[idx] < src2_aligned[idx]) ? 1 : 0;  /* LT_EXPR */
            mask_le_aligned[idx] = (src2_aligned[idx] <= src4_aligned[idx]) ? 1 : 0; /* LE_EXPR */
            
            /* Additional comparisons with scalar */
            mask_gt_aligned[idx] |= (src3_aligned[idx] > 100) ? 2 : 0;
            mask_ge_aligned[idx] |= (src4_aligned[idx] >= -500) ? 2 : 0;
            mask_lt_aligned[idx] |= (src1_aligned[idx] < 50) ? 2 : 0;
            mask_le_aligned[idx] |= (src2_aligned[idx] <= 1000) ? 2 : 0;
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i += 2) {  /* Non-unit stride */
            total_checksum += mask_gt_aligned[i] + mask_ge_aligned[i] 
                           + mask_lt_aligned[i] + mask_le_aligned[i];
        }
        
        /* Modify source data slightly for next iteration */
        for (int i = 0; i < N; i++) {
            src3_aligned[i] += (i % 3) - 1;
            src4_aligned[i] += (i % 5) - 2;
        }
    }
    
    /* Additional nested loop with mixed comparisons */
    for (int i = 0; i < N/2; i++) {
        for (int j = 0; j < 8; j++) {  /* Small inner loop */
            int idx = i*2 + j/4;
            
            /* Conditional select using comparisons */
            int temp_gt = (src3_aligned[idx] > src2_aligned[idx]) 
                         ? src3_aligned[idx] : src2_aligned[idx];
            int temp_ge = (src4_aligned[idx] >= src3_aligned[idx])
                         ? src4_aligned[idx] : src3_aligned[idx];
            int temp_lt = (src1_aligned[idx] < src2_aligned[idx])
                         ? src1_aligned[idx] : src2_aligned[idx];
            int temp_le = (src2_aligned[idx] <= src4_aligned[idx])
                         ? src2_aligned[idx] : src4_aligned[idx];
            
            mask_gt_aligned[idx] ^= temp_gt;
            mask_ge_aligned[idx] ^= temp_ge;
            mask_lt_aligned[idx] ^= temp_lt;
            mask_le_aligned[idx] ^= temp_le;
        }
    }
    
    /* Final checksum */
    for (int i = 0; i < N; i++) {
        total_checksum += mask_gt_aligned[i] + mask_ge_aligned[i]
                       + mask_lt_aligned[i] + mask_le_aligned[i];
    }
    
    printf("Final checksum: %ld\n", total_checksum);
    return (int)(total_checksum % 256);
}
