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
    
    /* Use volatile to prevent compile-time optimization */
    volatile int outer_bound = OUTER_ITER;
    volatile int use_arg = (argc > 1) ? atoi(argv[1]) : 42;
    uint32_t seed = use_arg;
    
    /* Initialize source arrays with patterned data */
    for (int i = 0; i < N; i++) {
        uint32_t r = simple_rand(&seed);
        src1_char[i] = (char)(r % 256 - 128);          /* -128 to 127 */
        src2_short[i] = (short)(r % 65536 - 32768);    /* -32768 to 32767 */
        src3_int[i] = (int)(r % 1000 - 500);           /* -500 to 499 */
        src4_long[i] = (long)(r % 2000 - 1000);        /* -1000 to 999 */
    }
    
    /* Provide alignment hints to the compiler */
    char *src1_aligned = __builtin_assume_aligned(src1_char, 32);
    short *src2_aligned = __builtin_assume_aligned(src2_short, 32);
    int *src3_aligned = __builtin_assume_aligned(src3_int, 32);
    long *src4_aligned = __builtin_assume_aligned(src4_long, 32);
    int *mask_gt_aligned = __builtin_assume_aligned(mask_gt, 32);
    int *mask_ge_aligned = __builtin_assume_aligned(mask_ge, 32);
    int *mask_lt_aligned = __builtin_assume_aligned(mask_lt, 32);
    int *mask_le_aligned = __builtin_assume_aligned(mask_le, 32);
    
    long long total_checksum = 0;
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride for complex patterns */
            int idx = i;
            
            /* GT_EXPR: char comparison */
            mask_gt_aligned[idx] = (src1_aligned[idx] > (char)(outer + 10)) ? 1 : 0;
            
            /* GE_EXPR: short comparison */
            mask_ge_aligned[idx] = (src2_aligned[idx] >= (short)(outer - 5)) ? 1 : 0;
            
            /* LT_EXPR: int comparison */
            mask_lt_aligned[idx] = (src3_aligned[idx] < (int)(outer * 2)) ? 1 : 0;
            
            /* LE_EXPR: long comparison */
            mask_le_aligned[idx] = (src4_aligned[idx] <= (long)(outer * 3)) ? 1 : 0;
        }
        
        /* Additional loop with array-to-array comparisons */
        for (int i = 0; i < N/2; i++) {
            /* Access with stride 2 for non-unit stride patterns */
            int idx = i * 2;
            
            /* All four comparisons between two arrays */
            mask_gt_aligned[idx] = (src1_aligned[idx] > src1_aligned[idx + 1]) ? 1 : 0;
            mask_ge_aligned[idx] = (src2_aligned[idx] >= src2_aligned[idx + 1]) ? 1 : 0;
            mask_lt_aligned[idx] = (src3_aligned[idx] < src3_aligned[idx + 1]) ? 1 : 0;
            mask_le_aligned[idx] = (src4_aligned[idx] <= src4_aligned[idx + 1]) ? 1 : 0;
        }
        
        /* Use conditional select operations */
        for (int i = 0; i < N; i++) {
            /* Conditional selects using comparison results */
            int temp1 = (src1_aligned[i] > src2_aligned[i % N]) ? 
                       src3_aligned[i] : src4_aligned[i % N];
            int temp2 = (src2_aligned[i] >= src3_aligned[i % N]) ? 
                       src4_aligned[i] : src1_aligned[i % N];
            int temp3 = (src3_aligned[i] < src4_aligned[i % N]) ? 
                       src1_aligned[i] : src2_aligned[i % N];
            int temp4 = (src4_aligned[i] <= src1_aligned[i % N]) ? 
                       src2_aligned[i] : src3_aligned[i % N];
            
            /* Mix results into masks to prevent elimination */
            mask_gt_aligned[i] ^= temp1;
            mask_ge_aligned[i] ^= temp2;
            mask_lt_aligned[i] ^= temp3;
            mask_le_aligned[i] ^= temp4;
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt_aligned[i] + mask_ge_aligned[i] + 
                            mask_lt_aligned[i] + mask_le_aligned[i];
        }
        
        /* Modify source data slightly for next outer iteration */
        for (int i = 0; i < N; i++) {
            src1_aligned[i] += (char)(i % 3);
            src2_aligned[i] += (short)(i % 5);
            src3_aligned[i] += (int)(i % 7);
            src4_aligned[i] += (long)(i % 11);
        }
    }
    
    /* Final observable side effect */
    printf("Final checksum: %lld\n", total_checksum);
    
    return (total_checksum != 0) ? 0 : 1;
}
