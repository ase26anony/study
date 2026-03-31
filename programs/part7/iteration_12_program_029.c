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
    
    /* Destination arrays for comparison results */
    int mask_gt[N] __attribute__((aligned(32)));
    int mask_ge[N] __attribute__((aligned(32)));
    int mask_lt[N] __attribute__((aligned(32)));
    int mask_le[N] __attribute__((aligned(32)));
    
    /* Initialize source arrays with patterned data */
    for (int i = 0; i < N; i++) {
        unsigned int r = simple_rand();
        src1_char[i] = (char)(r % 256 - 128);
        src1_short[i] = (short)(r % 65536 - 32768);
        src1_int[i] = (int)(r % 65536 - 32768);
        src1_long[i] = (long)(r % 65536 - 32768);
        
        r = simple_rand();
        src2_char[i] = (char)(r % 256 - 128);
        src2_short[i] = (short)(r % 65536 - 32768);
        src2_int[i] = (int)(r % 65536 - 32768);
        src2_long[i] = (long)(r % 65536 - 32768);
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
    
    volatile int outer_bound = OUTER_ITER;
    long long total_checksum = 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride 2 for complex patterns */
            int idx = (i * 2) % N;
            
            /* GT_EXPR: > comparison */
            p_gt[idx] = (p1_int[i] > p2_int[i]) ? 1 : 0;
            
            /* GE_EXPR: >= comparison */
            p_ge[idx] = (p1_short[i] >= p2_short[i]) ? 1 : 0;
            
            /* LT_EXPR: < comparison */
            p_lt[idx] = (p1_char[i] < p2_char[i]) ? 1 : 0;
            
            /* LE_EXPR: <= comparison */
            p_le[idx] = (p1_long[i] <= p2_long[i]) ? 1 : 0;
            
            /* Additional comparisons with scalar */
            p_gt[i] |= (p1_int[i] > 0) ? 1 : 0;
            p_ge[i] |= (p1_short[i] >= -100) ? 1 : 0;
            p_lt[i] |= (p1_char[i] < 50) ? 1 : 0;
            p_le[i] |= (p1_long[i] <= 1000) ? 1 : 0;
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += p_gt[i] + p_ge[i] + p_lt[i] + p_le[i];
        }
        
        /* Modify source data slightly each outer iteration */
        for (int i = 0; i < N; i += 4) {
            p1_int[i] += outer;
            p2_int[i] -= outer;
        }
    }
    
    /* Additional loop with mixed comparisons in conditional select */
    int result[N] __attribute__((aligned(32)));
    int *p_result = __builtin_assume_aligned(result, 32);
    
    for (int i = 0; i < N; i++) {
        /* Conditional select using comparisons */
        p_result[i] = (p1_int[i] > p2_int[i]) ? p1_int[i] : p2_int[i];
        p_result[i] = (p1_int[i] >= p2_int[i]) ? p_result[i] + 1 : p_result[i] - 1;
        p_result[i] = (p1_int[i] < p2_int[i]) ? p_result[i] * 2 : p_result[i] / 2;
        p_result[i] = (p1_int[i] <= p2_int[i]) ? p_result[i] ^ 0xFF : p_result[i] & 0xFF;
        
        total_checksum += p_result[i];
    }
    
    printf("Final checksum: %lld\n", total_checksum);
    return (int)(total_checksum % 256);
}
