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
    
    volatile int outer_bound = OUTER_ITER;  /* Prevent outer loop unrolling */
    uint32_t seed = 12345;
    
    /* Initialize arrays with pattern to avoid compile-time propagation */
    for (int i = 0; i < N; i++) {
        uint32_t r = simple_rand(&seed);
        src1_char[i] = (char)(r % 256 - 128);
        src2_short[i] = (short)(r % 65536 - 32768);
        src3_int[i] = (int)(r % 1000 - 500);
        src4_long[i] = (long)(r % 2000 - 1000);
        src_strided[i*2] = (int)(r % 1000);
    }
    
    /* Use argv to prevent constant propagation */
    int offset = (argc > 1) ? atoi(argv[1]) : 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Provide alignment hints to the compiler */
            char *aligned_char = __builtin_assume_aligned(src1_char, 32);
            short *aligned_short = __builtin_assume_aligned(src2_short, 32);
            int *aligned_int = __builtin_assume_aligned(src3_int, 32);
            long *aligned_long = __builtin_assume_aligned(src4_long, 32);
            
            /* GT_EXPR: > comparison */
            mask_gt[i] = (aligned_char[i] > (char)(i + offset)) ? 1 : 0;
            
            /* GE_EXPR: >= comparison with different type */
            mask_ge[i] = (aligned_short[i] >= (short)(i * 2 + offset)) ? 1 : 0;
            
            /* LT_EXPR: < comparison with scalar */
            mask_lt[i] = (aligned_int[i] < 100 + offset) ? 1 : 0;
            
            /* LE_EXPR: <= comparison */
            mask_le[i] = (aligned_long[i] <= (long)(i * 3 + offset)) ? 1 : 0;
            
            /* Additional comparison with strided access */
            int *aligned_strided = __builtin_assume_aligned(src_strided, 32);
            mask_strided[i] = (aligned_strided[i*2] <= aligned_strided[i*2 + 1]) ? 1 : 0;
            
            /* Mix comparisons with conditional select operations */
            int temp_gt = (aligned_int[i] > aligned_int[(i+1)%N]) ? 
                         aligned_int[i] : aligned_int[(i+1)%N];
            int temp_lt = (aligned_int[i] < aligned_int[(i+2)%N]) ? 
                         aligned_int[i] : aligned_int[(i+2)%N];
            
            /* Use results to prevent elimination */
            mask_gt[i] ^= temp_gt;
            mask_lt[i] ^= temp_lt;
        }
        
        /* Additional loop with mixed comparisons on same data */
        for (int i = 0; i < N - 1; i += 2) {  /* Even step for vectorization */
            /* All four comparisons on same data pair */
            int cmp1 = (src3_int[i] > src3_int[i+1]) ? 1 : 0;    /* GT_EXPR */
            int cmp2 = (src3_int[i] >= src3_int[i+1]) ? 1 : 0;   /* GE_EXPR */
            int cmp3 = (src3_int[i] < src3_int[i+1]) ? 1 : 0;    /* LT_EXPR */
            int cmp4 = (src3_int[i] <= src3_int[i+1]) ? 1 : 0;   /* LE_EXPR */
            
            /* Combine results */
            mask_gt[i] += cmp1 + cmp3;
            mask_ge[i] += cmp2 + cmp4;
        }
    }
    
    /* Compute checksum to ensure side effects are observable */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += mask_gt[i] + mask_ge[i] + mask_lt[i] + mask_le[i] + mask_strided[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    return (checksum > 0) ? 0 : 1;
}
