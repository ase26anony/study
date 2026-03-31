#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define OUTER_ITER 10

/* Simple PRNG to generate non-constant data */
static unsigned int seed = 12345;
static inline unsigned int simple_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time propagation */
    volatile int use_arg = argc > 1 ? atoi(argv[1]) : 1;
    
    /* Declare arrays with different integer types */
    char src1_char[N] __attribute__((aligned(32)));
    short src2_short[N] __attribute__((aligned(32)));
    int src3_int[N] __attribute__((aligned(32)));
    long src4_long[N] __attribute__((aligned(32)));
    
    /* Destination arrays for comparison results */
    int mask_gt[N] __attribute__((aligned(32)));
    int mask_ge[N] __attribute__((aligned(32)));
    int mask_lt[N] __attribute__((aligned(32)));
    int mask_le[N] __attribute__((aligned(32)));
    
    /* Initialize source arrays with patterned data */
    for (int i = 0; i < N; i++) {
        src1_char[i] = (char)((simple_rand() % 256) - 128);
        src2_short[i] = (short)((simple_rand() % 65536) - 32768);
        src3_int[i] = (int)(simple_rand() % 1000);
        src4_long[i] = (long)(simple_rand() % 1000);
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
    
    volatile int outer_bound = OUTER_ITER;
    long total_checksum = 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride (every other element) */
            int idx = (i * 2) % N;
            
            /* GT_EXPR: char comparison */
            mask_gt_aligned[i] = (src1_aligned[idx] > (char)(use_arg * 10)) ? 1 : 0;
            
            /* GE_EXPR: short comparison */
            mask_ge_aligned[i] = (src2_aligned[idx] >= (short)(use_arg * 20)) ? 1 : 0;
            
            /* LT_EXPR: int comparison */
            mask_lt_aligned[i] = (src3_aligned[idx] < (int)(use_arg * 30)) ? 1 : 0;
            
            /* LE_EXPR: long comparison */
            mask_le_aligned[i] = (src4_aligned[idx] <= (long)(use_arg * 40)) ? 1 : 0;
            
            /* Additional comparisons with array-to-array operations */
            if (i > 0) {
                /* GT_EXPR: array vs array */
                mask_gt_aligned[i] |= (src1_aligned[idx] > src1_aligned[(idx + 1) % N]) ? 2 : 0;
                
                /* GE_EXPR: array vs array */
                mask_ge_aligned[i] |= (src2_aligned[idx] >= src2_aligned[(idx + 1) % N]) ? 2 : 0;
                
                /* LT_EXPR: array vs array */
                mask_lt_aligned[i] |= (src3_aligned[idx] < src3_aligned[(idx + 1) % N]) ? 2 : 0;
                
                /* LE_EXPR: array vs array */
                mask_le_aligned[i] |= (src4_aligned[idx] <= src4_aligned[(idx + 1) % N]) ? 2 : 0;
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt_aligned[i] + mask_ge_aligned[i] 
                           + mask_lt_aligned[i] + mask_le_aligned[i];
        }
        
        /* Conditional select operations using comparison results */
        int temp_results[N];
        for (int i = 0; i < N; i++) {
            /* Use comparisons in conditional select */
            temp_results[i] = (src1_aligned[i] > (char)(use_arg * 5)) 
                            ? src3_aligned[i % N] 
                            : -src3_aligned[i % N];
            
            temp_results[i] += (src2_aligned[i] >= (short)(use_arg * 15))
                             ? src3_aligned[(i + 1) % N]
                             : -src3_aligned[(i + 1) % N];
            
            temp_results[i] += (src3_aligned[i] < (int)(use_arg * 25))
                             ? src4_aligned[i % N]
                             : -src4_aligned[i % N];
            
            temp_results[i] += (src4_aligned[i] <= (long)(use_arg * 35))
                             ? src3_aligned[(i + 2) % N]
                             : -src3_aligned[(i + 2) % N];
            
            total_checksum += temp_results[i];
        }
    }
    
    printf("Final checksum: %ld\n", total_checksum);
    return 0;
}
