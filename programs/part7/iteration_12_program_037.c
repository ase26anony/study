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
    volatile int init_val = argc > 1 ? atoi(argv[1]) : 42;
    
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
        unsigned int r = simple_rand();
        src1_char[i] = (char)((r % 256) - 128);
        src1_short[i] = (short)((r % 65536) - 32768);
        src1_int[i] = (int)(r % 1000) - 500;
        src1_long[i] = (long)(r % 2000) - 1000;
        
        r = simple_rand();
        src2_char[i] = (char)((r % 256) - 128);
        src2_short[i] = (short)((r % 65536) - 32768);
        src2_int[i] = (int)(r % 1000) - 500;
        src2_long[i] = (long)(r % 2000) - 1000;
    }
    
    /* Provide alignment hints to the compiler */
    char * __restrict__ p1c = src1_char;
    char * __restrict__ p2c = src2_char;
    short * __restrict__ p1s = src1_short;
    short * __restrict__ p2s = src2_short;
    int * __restrict__ p1i = src1_int;
    int * __restrict__ p2i = src2_int;
    long * __restrict__ p1l = src1_long;
    long * __restrict__ p2l = src2_long;
    
    __builtin_assume_aligned(p1c, 32);
    __builtin_assume_aligned(p2c, 32);
    __builtin_assume_aligned(p1s, 32);
    __builtin_assume_aligned(p2s, 32);
    __builtin_assume_aligned(p1i, 32);
    __builtin_assume_aligned(p2i, 32);
    __builtin_assume_aligned(p1l, 32);
    __builtin_assume_aligned(p2l, 32);
    
    volatile int outer_bound = OUTER_ITER;
    long long total_checksum = 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride (i*2) for complex patterns */
            int idx = (i * 2) % N;
            
            /* GT_EXPR: a > b */
            mask_gt[i] = (p1i[idx] > p2i[idx]) ? 1 : 0;
            
            /* GE_EXPR: a >= b */
            mask_ge[i] = (p1s[idx] >= p2s[idx]) ? 1 : 0;
            
            /* LT_EXPR: a < b */
            mask_lt[i] = (p1c[idx] < p2c[idx]) ? 1 : 0;
            
            /* LE_EXPR: a <= b */
            mask_le[i] = (p1l[idx] <= p2l[idx]) ? 1 : 0;
            
            /* Additional comparisons with scalar */
            mask_gt[i] |= (p1i[i] > init_val) ? 2 : 0;
            mask_ge[i] |= (p1s[i] >= (short)init_val) ? 2 : 0;
            mask_lt[i] |= (p1c[i] < (char)init_val) ? 2 : 0;
            mask_le[i] |= (p1l[i] <= (long)init_val) ? 2 : 0;
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt[i] + mask_ge[i] + mask_lt[i] + mask_le[i];
        }
        
        /* Conditional select operations using comparison results */
        int temp_results[N];
        for (int i = 0; i < N; i++) {
            /* Use comparisons in conditional select */
            temp_results[i] = (p1i[i] > p2i[i]) ? p1i[i] : p2i[i];
            temp_results[i] += (p1s[i] >= p2s[i]) ? p1s[i] : p2s[i];
            temp_results[i] += (p1c[i] < p2c[i]) ? p1c[i] : p2c[i];
            temp_results[i] += (p1l[i] <= p2l[i]) ? p1l[i] : p2l[i];
            total_checksum += temp_results[i];
        }
        
        /* Another loop with mixed-width comparisons */
        char mask_char[N];
        short mask_short[N];
        for (int i = 0; i < N; i += 2) {
            /* Mixed type comparisons */
            mask_char[i] = (p1c[i] > p2c[i]) ? 1 : 0;
            mask_short[i/2] = (p1s[i] >= p2s[i]) ? 1 : 0;
            
            /* Comparisons with different operand widths */
            mask_gt[i] = (p1i[i] < p2i[i]) ? 3 : 0;  /* LT_EXPR with int */
            mask_le[i] = (p1c[i] <= p2c[i]) ? 3 : 0; /* LE_EXPR with char */
        }
    }
    
    printf("Final checksum: %lld\n", total_checksum);
    return 0;
}
