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
        unsigned int val = simple_rand();
        src1_char[i] = (char)(val % 256 - 128);
        src1_short[i] = (short)(val % 65536 - 32768);
        src1_int[i] = (int)(val % 65536 - 32768);
        src1_long[i] = (long)(val % 65536 - 32768);
        
        val = simple_rand();
        src2_char[i] = (char)(val % 256 - 128);
        src2_short[i] = (short)(val % 65536 - 32768);
        src2_int[i] = (int)(val % 65536 - 32768);
        src2_long[i] = (long)(val % 65536 - 32768);
    }
    
    /* Provide alignment hints to the compiler */
    char *p1_char = __builtin_assume_aligned(src1_char, 32);
    short *p1_short = __builtin_assume_aligned(src1_short, 32);
    int *p1_int = __builtin_assume_aligned(src1_int, 32);
    long *p1_long = __builtin_assume_aligned(src1_long, 32);
    
    char *p2_char = __builtin_assume_aligned(src2_char, 32);
    short *p2_short = __builtin_assume_aligned(src2_short, 32);
    int *p2_int = __builtin_assume_aligned(src2_int, 32);
    long *p2_long = __builtin_assume_aligned(src2_long, 32);
    
    int *p_gt = __builtin_assume_aligned(mask_gt, 32);
    int *p_ge = __builtin_assume_aligned(mask_ge, 32);
    int *p_lt = __builtin_assume_aligned(mask_lt, 32);
    int *p_le = __builtin_assume_aligned(mask_le, 32);
    
    volatile int outer_bound = OUTER_ITER * use_arg;
    unsigned long long total_checksum = 0;
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride (every other element) */
            int idx = (i * 2) % N;
            
            /* Perform all four comparison types on different data types */
            
            /* GT_EXPR: char comparison */
            p_gt[idx] = (p1_char[idx] > p2_char[idx]) ? 1 : 0;
            
            /* GE_EXPR: short comparison */
            p_ge[idx] = (p1_short[idx] >= p2_short[idx]) ? 1 : 0;
            
            /* LT_EXPR: int comparison */
            p_lt[idx] = (p1_int[idx] < p2_int[idx]) ? 1 : 0;
            
            /* LE_EXPR: long comparison */
            p_le[idx] = (p1_long[idx] <= p2_long[idx]) ? 1 : 0;
            
            /* Additional comparisons with scalar values */
            if (i % 4 == 0) {
                /* GT_EXPR with scalar */
                p_gt[idx] |= (p1_char[idx] > 0) ? 2 : 0;
                
                /* GE_EXPR with scalar */
                p_ge[idx] |= (p1_short[idx] >= 100) ? 2 : 0;
                
                /* LT_EXPR with scalar */
                p_lt[idx] |= (p1_int[idx] < -100) ? 2 : 0;
                
                /* LE_EXPR with scalar */
                p_le[idx] |= (p1_long[idx] <= 100) ? 2 : 0;
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += p_gt[i] + p_ge[i] + p_lt[i] + p_le[i];
        }
        
        /* Conditional select operations using comparison results */
        int temp_results[N] __attribute__((aligned(32)));
        int *p_temp = __builtin_assume_aligned(temp_results, 32);
        
        for (int i = 0; i < N; i++) {
            /* Use comparison results in conditional selects */
            p_temp[i] = (p1_int[i] > p2_int[i]) ? p1_int[i] : p2_int[i];
            p_temp[i] += (p1_int[i] >= p2_int[i]) ? 1 : -1;
            p_temp[i] += (p1_int[i] < p2_int[i]) ? p1_int[i] : 0;
            p_temp[i] += (p1_int[i] <= p2_int[i]) ? p2_int[i] : 0;
            
            total_checksum += p_temp[i];
        }
    }
    
    printf("Final checksum: %llu\n", total_checksum);
    return 0;
}
