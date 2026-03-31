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
    
    /* Initialize with patterned data */
    uint32_t seed = 42;
    for (int i = 0; i < N; i++) {
        src1_char[i] = (char)(simple_rand(&seed) % 256 - 128);
        src2_short[i] = (short)(simple_rand(&seed) % 65536 - 32768);
        src3_int[i] = (int)(simple_rand(&seed) % 1000 - 500);
        src4_long[i] = (long)(simple_rand(&seed) % 2000 - 1000);
    }
    
    /* Provide alignment hints to the compiler */
    char *aligned_char = __builtin_assume_aligned(src1_char, 32);
    short *aligned_short = __builtin_assume_aligned(src2_short, 32);
    int *aligned_int = __builtin_assume_aligned(src3_int, 32);
    long *aligned_long = __builtin_assume_aligned(src4_long, 32);
    int *aligned_mask_gt = __builtin_assume_aligned(mask_gt, 32);
    int *aligned_mask_ge = __builtin_assume_aligned(mask_ge, 32);
    int *aligned_mask_lt = __builtin_assume_aligned(mask_lt, 32);
    int *aligned_mask_le = __builtin_assume_aligned(mask_le, 32);
    
    volatile int outer_bound = OUTER_ITER;
    long total_checksum = 0;
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Key inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride to create pattern */
            int idx = i;
            
            /* All four comparison operators on different data types */
            aligned_mask_gt[idx] = (aligned_char[idx] > (char)(use_arg)) ? 1 : 0;      /* GT_EXPR */
            aligned_mask_ge[idx] = (aligned_short[idx] >= (short)(use_arg + 1)) ? 1 : 0; /* GE_EXPR */
            aligned_mask_lt[idx] = (aligned_int[idx] < (int)(use_arg * 2)) ? 1 : 0;     /* LT_EXPR */
            aligned_mask_le[idx] = (aligned_long[idx] <= (long)(use_arg * 3)) ? 1 : 0;  /* LE_EXPR */
            
            /* Additional comparisons with array-to-array operations */
            if (i > 0) {
                /* Cross-type comparisons to stress conversion logic */
                aligned_mask_gt[idx] |= (aligned_char[idx] > aligned_char[idx-1]) ? 2 : 0;
                aligned_mask_ge[idx] |= (aligned_short[idx] >= aligned_short[idx-1]) ? 2 : 0;
                aligned_mask_lt[idx] |= (aligned_int[idx] < aligned_int[idx-1]) ? 2 : 0;
                aligned_mask_le[idx] |= (aligned_long[idx] <= aligned_long[idx-1]) ? 2 : 0;
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i += 2) {  /* Non-unit stride access */
            total_checksum += aligned_mask_gt[i] - aligned_mask_ge[i] 
                            + aligned_mask_lt[i] - aligned_mask_le[i];
        }
        
        /* Modify source data slightly each outer iteration */
        for (int i = 0; i < N; i++) {
            aligned_char[i] += (i % 3) - 1;
            aligned_short[i] += (i % 5) - 2;
            aligned_int[i] += (i % 7) - 3;
            aligned_long[i] += (i % 11) - 5;
        }
    }
    
    /* Additional loop with mixed comparisons in conditional select */
    int select_results[N];
    int *aligned_select = __builtin_assume_aligned(select_results, 32);
    
    for (int i = 0; i < N; i++) {
        /* Use comparisons in conditional select operations */
        aligned_select[i] = (aligned_char[i] > 0) ? aligned_int[i] : -aligned_int[i];
        aligned_select[i] += (aligned_short[i] >= 0) ? aligned_int[i] : 0;
        aligned_select[i] -= (aligned_int[i] < 0) ? aligned_int[i] : aligned_int[i]/2;
        aligned_select[i] *= (aligned_long[i] <= 0) ? 1 : 2;
        
        total_checksum += aligned_select[i];
    }
    
    printf("Final checksum: %ld\n", total_checksum);
    return (total_checksum != 0) ? 0 : 1;
}
