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
    /* Use volatile to prevent compile-time propagation */
    volatile int init_seed = (argc > 1) ? atoi(argv[1]) : 42;
    uint32_t seed = (uint32_t)init_seed;
    
    /* Declare arrays with different integer types */
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
    
    /* Initialize source arrays with patterned data */
    for (int i = 0; i < N; i++) {
        uint32_t val = simple_rand(&seed);
        src1_char[i] = (char)(val & 0xFF);
        src1_short[i] = (short)(val & 0xFFFF);
        src1_int[i] = (int)val;
        src1_long[i] = (long)val;
        
        val = simple_rand(&seed);
        src2_char[i] = (char)(val & 0xFF);
        src2_short[i] = (short)(val & 0xFFFF);
        src2_int[i] = (int)val;
        src2_long[i] = (long)val;
    }
    
    /* Provide alignment hints to the compiler */
    char * __restrict p1c = src1_char;
    short * __restrict p1s = src1_short;
    int * __restrict p1i = src1_int;
    long * __restrict p1l = src1_long;
    
    char * __restrict p2c = src2_char;
    short * __restrict p2s = src2_short;
    int * __restrict p2i = src2_int;
    long * __restrict p2l = src2_long;
    
    int * __restrict pmask_gt = mask_gt;
    int * __restrict pmask_ge = mask_ge;
    int * __restrict pmask_lt = mask_lt;
    int * __restrict pmask_le = mask_le;
    
    __builtin_assume_aligned(p1c, 32);
    __builtin_assume_aligned(p1s, 32);
    __builtin_assume_aligned(p1i, 32);
    __builtin_assume_aligned(p1l, 32);
    __builtin_assume_aligned(p2c, 32);
    __builtin_assume_aligned(p2s, 32);
    __builtin_assume_aligned(p2i, 32);
    __builtin_assume_aligned(p2l, 32);
    __builtin_assume_aligned(pmask_gt, 32);
    __builtin_assume_aligned(pmask_ge, 32);
    __builtin_assume_aligned(pmask_lt, 32);
    __builtin_assume_aligned(pmask_le, 32);
    
    volatile int outer_bound = OUTER_ITER;
    long long total_checksum = 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Use different data types for different comparisons */
            /* GT_EXPR: > comparison */
            pmask_gt[i] = (p1_int[i] > p2_int[i]) ? 1 : 0;
            
            /* GE_EXPR: >= comparison */
            pmask_ge[i] = (p1_long[i] >= p2_long[i]) ? 1 : 0;
            
            /* LT_EXPR: < comparison */
            pmask_lt[i] = (p1_short[i] < p2_short[i]) ? 1 : 0;
            
            /* LE_EXPR: <= comparison */
            pmask_le[i] = (p1_char[i] <= p2_char[i]) ? 1 : 0;
            
            /* Additional comparisons with scalar */
            pmask_gt[i] |= (p1_int[i] > 1000) ? 2 : 0;
            pmask_ge[i] |= (p1_long[i] >= -500L) ? 2 : 0;
            pmask_lt[i] |= (p1_short[i] < 30000) ? 2 : 0;
            pmask_le[i] |= (p1_char[i] <= 100) ? 2 : 0;
        }
        
        /* Non-constant stride access pattern */
        for (int i = 0; i < N/2; i++) {
            int idx = i * 2;  /* Constant stride of 2 */
            pmask_gt[idx] += (p1_int[idx] > p2_int[idx+1]) ? 4 : 0;
            pmask_ge[idx] += (p1_long[idx] >= p2_long[idx+1]) ? 4 : 0;
            pmask_lt[idx] += (p1_short[idx] < p2_short[idx+1]) ? 4 : 0;
            pmask_le[idx] += (p1_char[idx] <= p2_char[idx+1]) ? 4 : 0;
        }
        
        /* Conditional select operations using comparison results */
        int temp_results[N];
        for (int i = 0; i < N; i++) {
            /* Use comparisons in conditional select */
            temp_results[i] = (p1_int[i] > p2_int[i]) ? p1_int[i] : p2_int[i];
            temp_results[i] += (p1_long[i] >= p2_long[i]) ? 1 : -1;
            temp_results[i] *= (p1_short[i] < p2_short[i]) ? 2 : 1;
            temp_results[i] /= ((p1_char[i] <= p2_char[i]) ? 2 : 1) + 1;
        }
        
        /* Compute checksum to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += pmask_gt[i] + pmask_ge[i] + pmask_lt[i] + pmask_le[i];
            total_checksum += temp_results[i];
        }
        
        /* Modify source data slightly for next outer iteration */
        for (int i = 0; i < N; i++) {
            p1_int[i] += outer;
            p2_int[i] -= outer;
        }
    }
    
    printf("Final checksum: %lld\n", total_checksum);
    return 0;
}
