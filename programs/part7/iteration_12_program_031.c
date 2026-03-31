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
    /* Use argv to prevent compile-time propagation */
    int use_arg = (argc > 1) ? atoi(argv[1]) : 0;
    
    /* Declare arrays with different integer types */
    char src1_char[N];
    short src1_short[N];
    int src1_int[N];
    long src1_long[N];
    
    char src2_char[N];
    short src2_short[N];
    int src2_int[N];
    long src2_long[N];
    
    /* Destination mask arrays for comparison results */
    int mask_gt[N] __attribute__((aligned(32)));
    int mask_ge[N] __attribute__((aligned(32)));
    int mask_lt[N] __attribute__((aligned(32)));
    int mask_le[N] __attribute__((aligned(32)));
    
    /* Initialize source arrays with patterned data */
    for (int i = 0; i < N; i++) {
        unsigned int val = simple_rand();
        src1_char[i] = (char)(val % 256);
        src1_short[i] = (short)(val % 65536);
        src1_int[i] = (int)val;
        src1_long[i] = (long)val;
        
        val = simple_rand();
        src2_char[i] = (char)(val % 256);
        src2_short[i] = (short)(val % 65536);
        src2_int[i] = (int)val;
        src2_long[i] = (long)val;
    }
    
    /* Provide alignment hints */
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
    
    volatile int outer_bound = OUTER_ITER + use_arg;
    long long total_checksum = 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Use compile-time constant stride (i*2 mod N) for complex patterns */
            int idx = (i * 2) % N;
            
            /* GT_EXPR: > comparison with char type */
            p_gt[i] = (p1_char[idx] > p2_char[idx]) ? 1 : 0;
            
            /* GE_EXPR: >= comparison with short type */
            p_ge[i] = (p1_short[idx] >= p2_short[idx]) ? 1 : 0;
            
            /* LT_EXPR: < comparison with int type */
            p_lt[i] = (p1_int[idx] < p2_int[idx]) ? 1 : 0;
            
            /* LE_EXPR: <= comparison with long type */
            p_le[i] = (p1_long[idx] <= p2_long[idx]) ? 1 : 0;
            
            /* Additional comparisons with scalar values */
            if (i % 4 == 0) {
                /* Mixed comparisons with scalar */
                p_gt[i] |= (p1_int[i] > 1000) ? 2 : 0;
                p_ge[i] |= (p1_short[i] >= -500) ? 2 : 0;
                p_lt[i] |= (p1_char[i] < 50) ? 2 : 0;
                p_le[i] |= (p1_long[i] <= 1000000L) ? 2 : 0;
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += p_gt[i] + p_ge[i] + p_lt[i] + p_le[i];
            
            /* Conditional select based on comparison results */
            int temp = (p_gt[i] & 1) ? p1_int[i] : p2_int[i];
            total_checksum += temp;
            
            temp = (p_ge[i] & 1) ? p1_short[i] : p2_short[i];
            total_checksum += temp;
            
            temp = (p_lt[i] & 1) ? p1_char[i] : p2_char[i];
            total_checksum += temp;
            
            temp = (p_le[i] & 1) ? p1_long[i] : p2_long[i];
            total_checksum += temp;
        }
        
        /* Modify source data slightly each outer iteration */
        for (int i = 0; i < N; i++) {
            p1_char[i] += (char)(i % 3);
            p2_char[i] -= (char)(i % 5);
            p1_short[i] += (short)(i % 7);
            p2_short[i] -= (short)(i % 11);
        }
    }
    
    printf("Final checksum: %lld\n", total_checksum);
    return 0;
}
