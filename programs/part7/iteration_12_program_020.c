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
    /* Use volatile to prevent compile-time propagation */
    volatile int init_val = (argc > 1) ? atoi(argv[1]) : 42;
    
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
    
    /* Initialize arrays with patterned data */
    for (int i = 0; i < N; i++) {
        unsigned int r = simple_rand();
        src1_char[i] = (char)((r % 256) - 128);
        src2_short[i] = (short)((r % 65536) - 32768);
        src3_int[i] = (int)(r % 1000) - 500;
        src4_long[i] = (long)(r % 2000) - 1000;
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
            /* Access with constant stride (i*2) for complex patterns */
            int idx = (i * 2) % N;
            
            /* GT_EXPR: char comparison */
            mask_gt_aligned[i] = (src1_aligned[idx] > (char)(init_val + outer)) ? 1 : 0;
            
            /* GE_EXPR: short comparison */
            mask_ge_aligned[i] = (src2_aligned[idx] >= (short)(init_val - outer)) ? 1 : 0;
            
            /* LT_EXPR: int comparison */
            mask_lt_aligned[i] = (src3_aligned[i] < (int)(init_val * 2)) ? 1 : 0;
            
            /* LE_EXPR: long comparison */
            mask_le_aligned[i] = (src4_aligned[i] <= (long)(init_val * 3)) ? 1 : 0;
            
            /* Additional comparisons with mixed types */
            if (i % 4 == 0) {
                /* Cross-type comparisons to stress conversion logic */
                mask_gt_aligned[i] |= (src1_aligned[i] > src2_aligned[i]) ? 2 : 0;
                mask_ge_aligned[i] |= (src2_aligned[i] >= src3_aligned[i]) ? 2 : 0;
                mask_lt_aligned[i] |= (src3_aligned[i] < src4_aligned[i]) ? 2 : 0;
                mask_le_aligned[i] |= (src4_aligned[i] <= src1_aligned[i]) ? 2 : 0;
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt_aligned[i] + mask_ge_aligned[i] 
                           + mask_lt_aligned[i] + mask_le_aligned[i];
        }
        
        /* Modify source data slightly for next outer iteration */
        for (int i = 0; i < N; i += 4) {
            src1_aligned[i] += 1;
            src2_aligned[i] -= 1;
            src3_aligned[i] += outer;
            src4_aligned[i] -= outer;
        }
    }
    
    /* Conditional select operations using comparison results */
    int select_results[N];
    for (int i = 0; i < N; i++) {
        /* Use comparisons in conditional select */
        select_results[i] = (src3_aligned[i] > src1_aligned[i]) 
                          ? src3_aligned[i] 
                          : src1_aligned[i];
        select_results[i] = (src4_aligned[i] >= src2_aligned[i]) 
                          ? select_results[i] + src4_aligned[i] 
                          : select_results[i] - src2_aligned[i];
        select_results[i] = (src1_aligned[i] < (char)init_val) 
                          ? select_results[i] * 2 
                          : select_results[i] / 2;
        select_results[i] = (src2_aligned[i] <= (short)init_val) 
                          ? select_results[i] + i 
                          : select_results[i] - i;
    }
    
    /* Final checksum computation */
    long final_checksum = total_checksum;
    for (int i = 0; i < N; i++) {
        final_checksum += select_results[i];
    }
    
    printf("Final checksum: %ld\n", final_checksum);
    return (final_checksum > 0) ? 0 : 1;
}
