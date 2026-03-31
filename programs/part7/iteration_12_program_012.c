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
    /* Use argc to prevent compile-time optimization */
    uint32_t seed = (argc > 1) ? (uint32_t)atoi(argv[1]) : 123456;
    
    /* Declare source arrays with different integer types */
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
        uint32_t r = simple_rand(&seed);
        src1_char[i] = (char)(r % 256 - 128);           /* Signed char */
        src2_short[i] = (short)(r % 65536 - 32768);     /* Signed short */
        src3_int[i] = (int)(r % 65536 - 32768);         /* Signed int */
        src4_long[i] = (long)(r % 65536 - 32768);       /* Signed long */
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
    
    /* Volatile variable to control outer loop and prevent unrolling */
    volatile int outer_limit = OUTER_ITER;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_limit; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride (i*2) for complex patterns */
            int idx = (i * 2) % N;
            
            /* GT_EXPR: char comparison */
            mask_gt_aligned[idx] = (src1_aligned[idx] > (char)(i % 256 - 128)) ? 1 : 0;
            
            /* GE_EXPR: short comparison with scalar */
            mask_ge_aligned[idx] = (src2_aligned[idx] >= (short)(outer * 10)) ? 1 : 0;
            
            /* LT_EXPR: int comparison */
            mask_lt_aligned[idx] = (src3_aligned[idx] < src3_aligned[(idx + 1) % N]) ? 1 : 0;
            
            /* LE_EXPR: long comparison */
            mask_le_aligned[idx] = (src4_aligned[idx] <= src4_aligned[(idx + N/2) % N]) ? 1 : 0;
        }
        
        /* Additional loop with mixed comparisons on same data */
        for (int i = 0; i < N; i += 2) {  /* Non-unit stride */
            /* All four comparisons on the same data pair */
            int val1 = src3_aligned[i];
            int val2 = src3_aligned[i + 1];
            
            /* Store all comparison results */
            mask_gt_aligned[i] = (val1 > val2) ? -1 : 0;
            mask_ge_aligned[i] = (val1 >= val2) ? -1 : 0;
            mask_lt_aligned[i] = (val1 < val2) ? -1 : 0;
            mask_le_aligned[i] = (val1 <= val2) ? -1 : 0;
            
            /* Conditional select operations using comparisons */
            mask_gt_aligned[i + 1] = (val1 > 0) ? val1 : val2;
            mask_ge_aligned[i + 1] = (val1 >= 0) ? val1 : -val2;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += mask_gt[i] + mask_ge[i] + mask_lt[i] + mask_le[i];
        checksum += src1_char[i] + src2_short[i] + src3_int[i] + src4_long[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Additional test with nested loops for outer loop vectorization */
    {
        int temp[N] __attribute__((aligned(32)));
        int *temp_aligned = __builtin_assume_aligned(temp, 32);
        
        /* Initialize temp array */
        for (int i = 0; i < N; i++) {
            temp_aligned[i] = i - N/2;
        }
        
        /* Nested loop structure */
        for (int k = 0; k < 4; k++) {
            for (int i = 0; i < N; i++) {
                /* Mixed comparisons in nested loop */
                int cmp1 = (temp_aligned[i] > src3_aligned[i]) ? 1 : 0;
                int cmp2 = (temp_aligned[i] >= src3_aligned[i]) ? 1 : 0;
                int cmp3 = (temp_aligned[i] < src3_aligned[i]) ? 1 : 0;
                int cmp4 = (temp_aligned[i] <= src3_aligned[i]) ? 1 : 0;
                
                /* Use comparisons in arithmetic */
                mask_gt_aligned[i] = cmp1 * 2 + cmp2;
                mask_ge_aligned[i] = cmp3 * 3 + cmp4;
                
                /* Update temp with comparison-dependent values */
                temp_aligned[i] = (temp_aligned[i] > 0) ? 
                                 temp_aligned[i] * 2 : 
                                 temp_aligned[i] / 2;
            }
        }
        
        /* Final checksum */
        long long final_checksum = 0;
        for (int i = 0; i < N; i++) {
            final_checksum += temp[i] + mask_gt[i] + mask_ge[i];
        }
        printf("Final checksum: %lld\n", final_checksum);
    }
    
    return 0;
}
