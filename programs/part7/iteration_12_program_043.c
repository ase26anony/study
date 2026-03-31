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
    
    volatile int outer_bound = OUTER_ITER;
    uint32_t seed = 123456789;
    
    /* Initialize source arrays with pattern data */
    for (int i = 0; i < N; i++) {
        src1_char[i] = (char)(simple_rand(&seed) % 256 - 128);
        src2_short[i] = (short)(simple_rand(&seed) % 65536 - 32768);
        src3_int[i] = (int)(simple_rand(&seed) % 1000 - 500);
        src4_long[i] = (long)(simple_rand(&seed) % 2000 - 1000);
        src_strided[i*2] = (int)(simple_rand(&seed) % 1000);
    }
    
    /* Use __builtin_assume_aligned for alignment hints */
    char *aligned_char = __builtin_assume_aligned(src1_char, 32);
    short *aligned_short = __builtin_assume_aligned(src2_short, 32);
    int *aligned_int = __builtin_assume_aligned(src3_int, 32);
    long *aligned_long = __builtin_assume_aligned(src4_long, 32);
    int *aligned_strided = __builtin_assume_aligned(src_strided, 32);
    
    int checksum = 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* GT_EXPR: > comparison */
            mask_gt[i] = (aligned_char[i] > (char)(i % 256 - 128)) ? 1 : 0;
            
            /* GE_EXPR: >= comparison with different types */
            mask_ge[i] = (aligned_short[i] >= (short)(i % 1000)) ? 1 : 0;
            
            /* LT_EXPR: < comparison */
            mask_lt[i] = (aligned_int[i] < (int)(i * 2)) ? 1 : 0;
            
            /* LE_EXPR: <= comparison */
            mask_le[i] = (aligned_long[i] <= (long)(i * 3)) ? 1 : 0;
            
            /* Additional comparisons with constant stride */
            mask_strided[i] = (aligned_strided[i*2] > aligned_strided[i*2 + 1]) ? 1 : 0;
            
            /* Mixed comparisons to ensure all paths are taken */
            if (i % 4 == 0) {
                mask_gt[i] |= (aligned_int[i] > aligned_int[(i+1) % N]) ? 2 : 0;
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            checksum += mask_gt[i] + mask_ge[i] + mask_lt[i] + mask_le[i] + mask_strided[i];
        }
        
        /* Modify source data slightly for next iteration */
        for (int i = 0; i < N; i++) {
            aligned_char[i] += (i % 3) - 1;
            aligned_short[i] += (i % 5) - 2;
            aligned_int[i] += (i % 7) - 3;
            aligned_long[i] += (i % 11) - 5;
        }
    }
    
    /* Conditional select operations using comparison results */
    int select_results[N] __attribute__((aligned(32)));
    for (int i = 0; i < N; i++) {
        /* Use comparisons in conditional select */
        select_results[i] = (aligned_char[i] > 0) ? aligned_int[i] : -aligned_int[i];
        select_results[i] += (aligned_short[i] >= 0) ? aligned_int[(i+1) % N] : 0;
        select_results[i] += (aligned_int[i] < 0) ? i : -i;
        select_results[i] += (aligned_long[i] <= 0) ? (i * 2) : (i * 3);
        
        checksum += select_results[i];
    }
    
    /* Additional nested loop for outer-loop vectorization */
    int temp[N] __attribute__((aligned(32)));
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < N; i++) {
            /* All four comparisons in a single statement group */
            int cmp1 = aligned_char[i] > (char)j;
            int cmp2 = aligned_short[i] >= (short)(j * 10);
            int cmp3 = aligned_int[i] < (int)(j * 100);
            int cmp4 = aligned_long[i] <= (long)(j * 1000);
            
            temp[i] = cmp1 + cmp2 * 2 + cmp3 * 4 + cmp4 * 8;
            checksum += temp[i];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
