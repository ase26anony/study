#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define OUTER_ITER 10

/* Simple PRNG to generate non-constant data */
static unsigned int seed = 12345;
static unsigned int simple_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
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
    
    /* Initialize source arrays with patterned data */
    for (int i = 0; i < N; i++) {
        src1_char[i] = (char)((simple_rand() % 256) - 128);
        src2_short[i] = (short)((simple_rand() % 65536) - 32768);
        src3_int[i] = (int)(simple_rand() - 16384);
        src4_long[i] = (long)(simple_rand() * 100L - 1638400L);
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
            /* Access with constant stride for complex patterns */
            int idx = i;
            
            /* GT_EXPR: char comparison */
            mask_gt_aligned[idx] = (src1_aligned[idx] > (char)(init_val + outer)) ? 1 : 0;
            
            /* GE_EXPR: short comparison */
            mask_ge_aligned[idx] = (src2_aligned[idx] >= (short)(init_val - outer)) ? 1 : 0;
            
            /* LT_EXPR: int comparison - swapped operands case */
            mask_lt_aligned[idx] = ((init_val * 2) < src3_aligned[idx]) ? 1 : 0;
            
            /* LE_EXPR: long comparison - swapped operands case */
            mask_le_aligned[idx] = ((init_val * 3L) <= src4_aligned[idx]) ? 1 : 0;
            
            /* Additional comparisons with array-to-array operations */
            if (i > 0) {
                /* GT_EXPR: array vs array */
                mask_gt_aligned[idx] |= (src1_aligned[idx] > src1_aligned[idx-1]) ? 2 : 0;
                
                /* GE_EXPR: array vs array */
                mask_ge_aligned[idx] |= (src2_aligned[idx] >= src2_aligned[idx-1]) ? 2 : 0;
                
                /* LT_EXPR: array vs array with swapped operands */
                mask_lt_aligned[idx] |= (src3_aligned[idx-1] < src3_aligned[idx]) ? 2 : 0;
                
                /* LE_EXPR: array vs array with swapped operands */
                mask_le_aligned[idx] |= (src4_aligned[idx-1] <= src4_aligned[idx]) ? 2 : 0;
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i += 2) {  /* Non-unit stride consumption */
            total_checksum += mask_gt_aligned[i];
            total_checksum += mask_ge_aligned[i];
            total_checksum += mask_lt_aligned[i];
            total_checksum += mask_le_aligned[i];
        }
        
        /* Modify source data slightly each outer iteration */
        for (int i = 0; i < N; i++) {
            src1_aligned[i] += (char)(outer % 4);
            src2_aligned[i] += (short)(outer % 8);
            src3_aligned[i] += outer;
            src4_aligned[i] += outer * 10L;
        }
    }
    
    /* Additional loop with mixed comparisons in conditional select */
    int select_results[N];
    for (int i = 0; i < N; i++) {
        /* Conditional select using GT comparison */
        select_results[i] = (src1_aligned[i] > 0) ? src3_aligned[i] : -src3_aligned[i];
        
        /* Conditional select using GE comparison */
        select_results[i] += (src2_aligned[i] >= 0) ? src3_aligned[i] : 0;
        
        /* Conditional select using LT comparison (swapped) */
        select_results[i] += (0 < src3_aligned[i]) ? src4_aligned[i] % 100 : 0;
        
        /* Conditional select using LE comparison (swapped) */
        select_results[i] += (0 <= src4_aligned[i]) ? 1 : -1;
    }
    
    /* Final consumption */
    for (int i = 0; i < N; i++) {
        total_checksum += select_results[i];
    }
    
    printf("Final checksum: %ld\n", total_checksum);
    return 0;
}
