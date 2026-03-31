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
    
    /* Additional arrays for conditional select operations */
    int select_gt[N] __attribute__((aligned(32)));
    int select_ge[N] __attribute__((aligned(32)));
    int select_lt[N] __attribute__((aligned(32)));
    int select_le[N] __attribute__((aligned(32)));
    
    volatile int outer_bound = OUTER_ITER;
    uint32_t seed = 42;
    
    /* Initialize source arrays with patterned data */
    for (int i = 0; i < N; i++) {
        uint32_t r = simple_rand(&seed);
        src1_char[i] = (char)(r % 256);
        src1_short[i] = (short)(r % 65536);
        src1_int[i] = (int)r;
        src1_long[i] = (long)r;
        
        r = simple_rand(&seed);
        src2_char[i] = (char)(r % 256);
        src2_short[i] = (short)(r % 65536);
        src2_int[i] = (int)r;
        src2_long[i] = (long)r;
    }
    
    /* Use __builtin_assume_aligned for alignment hints */
    char *a_char = __builtin_assume_aligned(src1_char, 32);
    short *a_short = __builtin_assume_aligned(src1_short, 32);
    int *a_int = __builtin_assume_aligned(src1_int, 32);
    long *a_long = __builtin_assume_aligned(src1_long, 32);
    
    char *b_char = __builtin_assume_aligned(src2_char, 32);
    short *b_short = __builtin_assume_aligned(src2_short, 32);
    int *b_int = __builtin_assume_aligned(src2_int, 32);
    long *b_long = __builtin_assume_aligned(src2_long, 32);
    
    int *m_gt = __builtin_assume_aligned(mask_gt, 32);
    int *m_ge = __builtin_assume_aligned(mask_ge, 32);
    int *m_lt = __builtin_assume_aligned(mask_lt, 32);
    int *m_le = __builtin_assume_aligned(mask_le, 32);
    
    int *s_gt = __builtin_assume_aligned(select_gt, 32);
    int *s_ge = __builtin_assume_aligned(select_ge, 32);
    int *s_lt = __builtin_assume_aligned(select_lt, 32);
    int *s_le = __builtin_assume_aligned(select_le, 32);
    
    long long total_checksum = 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison operators */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride (i*2) for complex patterns */
            int idx = (i * 2) % N;
            
            /* GT_EXPR: a > b */
            m_gt[i] = (a_int[idx] > b_int[idx]) ? 1 : 0;
            
            /* GE_EXPR: a >= b */
            m_ge[i] = (a_short[idx] >= b_short[idx]) ? 1 : 0;
            
            /* LT_EXPR: a < b */
            m_lt[i] = (a_char[idx] < b_char[idx]) ? 1 : 0;
            
            /* LE_EXPR: a <= b */
            m_le[i] = (a_long[idx] <= b_long[idx]) ? 1 : 0;
            
            /* Conditional select operations using comparisons */
            s_gt[i] = (a_int[i] > b_int[i]) ? a_int[i] : b_int[i];
            s_ge[i] = (a_short[i] >= b_short[i]) ? a_short[i] : b_short[i];
            s_lt[i] = (a_char[i] < b_char[i]) ? a_char[i] : b_char[i];
            s_le[i] = (a_long[i] <= b_long[i]) ? a_long[i] : b_long[i];
            
            /* Additional comparisons with scalar */
            int scalar = outer * 100;
            m_gt[i] |= (a_int[i] > scalar) ? 2 : 0;
            m_ge[i] |= (a_short[i] >= scalar) ? 2 : 0;
            m_lt[i] |= (a_char[i] < scalar) ? 2 : 0;
            m_le[i] |= (a_long[i] <= scalar) ? 2 : 0;
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += m_gt[i] + m_ge[i] + m_lt[i] + m_le[i];
            total_checksum += s_gt[i] + s_ge[i] + s_lt[i] + s_le[i];
        }
        
        /* Modify source data slightly for next outer iteration */
        for (int i = 0; i < N; i++) {
            a_int[i] += outer;
            b_int[i] -= outer;
        }
    }
    
    printf("Final checksum: %lld\n", total_checksum);
    
    /* Additional test with nested loops for outer-loop vectorization */
    {
        int arr1[256][8] __attribute__((aligned(32)));
        int arr2[256][8] __attribute__((aligned(32)));
        int mask_arr[256][8] __attribute__((aligned(32)));
        
        /* Initialize */
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 8; j++) {
                arr1[i][j] = i * 8 + j;
                arr2[i][j] = (i * 8 + j) * 2;
            }
        }
        
        /* Outer loop vectorization test */
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 8; j++) {
                /* Mix of comparison operators */
                if (i % 4 == 0) {
                    mask_arr[i][j] = (arr1[i][j] > arr2[i][j]) ? 1 : 0;  /* GT_EXPR */
                } else if (i % 4 == 1) {
                    mask_arr[i][j] = (arr1[i][j] >= arr2[i][j]) ? 1 : 0; /* GE_EXPR */
                } else if (i % 4 == 2) {
                    mask_arr[i][j] = (arr1[i][j] < arr2[i][j]) ? 1 : 0;  /* LT_EXPR */
                } else {
                    mask_arr[i][j] = (arr1[i][j] <= arr2[i][j]) ? 1 : 0; /* LE_EXPR */
                }
            }
        }
        
        /* Consume results */
        int nested_checksum = 0;
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 8; j++) {
                nested_checksum += mask_arr[i][j];
            }
        }
        printf("Nested checksum: %d\n", nested_checksum);
    }
    
    return 0;
}
