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
        src3_int[i] = (int)(simple_rand(&seed) % 1000);
        src4_long[i] = (long)(simple_rand(&seed) % 2000 - 1000);
    }
    
    /* Provide alignment hints to the compiler */
    char * __restrict a1 = src1_char;
    short * __restrict a2 = src2_short;
    int * __restrict a3 = src3_int;
    long * __restrict a4 = src4_long;
    
    __builtin_assume_aligned(a1, 32);
    __builtin_assume_aligned(a2, 32);
    __builtin_assume_aligned(a3, 32);
    __builtin_assume_aligned(a4, 32);
    
    volatile int outer_bound = OUTER_ITER;
    long long total_checksum = 0;
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Key inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride to create pattern */
            int idx = i;
            
            /* GT_EXPR: greater than */
            mask_gt[idx] = (a3[i] > (int)(a4[i] + outer));
            
            /* GE_EXPR: greater than or equal */
            mask_ge[idx] = (a3[i] >= (int)(a4[i] - outer));
            
            /* LT_EXPR: less than - using different data types */
            mask_lt[idx] = ((long)a2[i] < a4[i]);
            
            /* LE_EXPR: less than or equal - with type conversion */
            mask_le[idx] = ((int)a1[i] <= a3[i]);
            
            /* Additional comparisons with scalar */
            mask_gt[idx] |= (a3[i] > 500);  // GT_EXPR with scalar
            mask_ge[idx] |= (a3[i] >= -500); // GE_EXPR with scalar
            mask_lt[idx] |= (a2[i] < 0);    // LT_EXPR with scalar
            mask_le[idx] |= (a1[i] <= 100); // LE_EXPR with scalar
        }
        
        /* Second inner loop with different access pattern */
        for (int i = 0; i < N/2; i++) {
            /* Access with stride 2 */
            int idx = i * 2;
            
            /* Mixed comparisons on same data */
            mask_gt[idx] = (a3[idx] > a3[idx+1]);
            mask_ge[idx] = (a3[idx] >= a3[idx+1]);
            mask_lt[idx] = (a3[idx] < a3[idx+1]);
            mask_le[idx] = (a3[idx] <= a3[idx+1]);
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt[i] + mask_ge[i] * 2 
                           + mask_lt[i] * 3 + mask_le[i] * 4;
        }
    }
    
    /* Conditional select operations using comparison results */
    int result[N];
    for (int i = 0; i < N; i++) {
        /* Use comparisons in conditional select pattern */
        result[i] = (a3[i] > 0) ? a3[i] : -a3[i];
        result[i] = (a3[i] >= a4[i]) ? result[i] : a4[i];
        result[i] = (a2[i] < 0) ? result[i] + a2[i] : result[i] - a2[i];
        result[i] = (a1[i] <= 0) ? result[i] * 2 : result[i] / 2;
        
        total_checksum += result[i];
    }
    
    printf("Checksum: %lld\n", total_checksum);
    return (total_checksum > 0) ? 0 : 1;
}
