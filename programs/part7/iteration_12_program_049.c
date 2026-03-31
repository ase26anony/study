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
    /* Use argc to prevent compile-time propagation */
    volatile int use_arg = argc > 1 ? atoi(argv[1]) : 1;
    
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
    
    /* Initialize with patterned data */
    uint32_t seed = 42;
    for (int i = 0; i < N; i++) {
        uint32_t val = simple_rand(&seed);
        src1_char[i] = (char)(val % 256 - 128);
        src1_short[i] = (short)(val % 65536 - 32768);
        src1_int[i] = (int)(val % 65536 - 32768);
        src1_long[i] = (long)(val % 65536 - 32768);
        
        val = simple_rand(&seed);
        src2_char[i] = (char)(val % 256 - 128);
        src2_short[i] = (short)(val % 65536 - 32768);
        src2_int[i] = (int)(val % 65536 - 32768);
        src2_long[i] = (long)(val % 65536 - 32768);
    }
    
    /* Provide alignment hints to the compiler */
    char * __restrict__ p1c = src1_char;
    short * __restrict__ p1s = src1_short;
    int * __restrict__ p1i = src1_int;
    long * __restrict__ p1l = src1_long;
    
    char * __restrict__ p2c = src2_char;
    short * __restrict__ p2s = src2_short;
    int * __restrict__ p2i = src2_int;
    long * __restrict__ p2l = src2_long;
    
    __builtin_assume_aligned(p1c, 32);
    __builtin_assume_aligned(p1s, 32);
    __builtin_assume_aligned(p1i, 32);
    __builtin_assume_aligned(p1l, 32);
    __builtin_assume_aligned(p2c, 32);
    __builtin_assume_aligned(p2s, 32);
    __builtin_assume_aligned(p2i, 32);
    __builtin_assume_aligned(p2l, 32);
    
    volatile int outer_bound = OUTER_ITER;
    long long total_checksum = 0;
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Use different data types for different comparisons */
            /* This stresses type conversion logic */
            
            /* GT_EXPR: char comparison */
            mask_gt[i] = (p1c[i] > p2c[i]) ? 1 : 0;
            
            /* GE_EXPR: short comparison */
            mask_ge[i] = (p1s[i] >= p2s[i]) ? 1 : 0;
            
            /* LT_EXPR: int comparison */
            mask_lt[i] = (p1i[i] < p2i[i]) ? 1 : 0;
            
            /* LE_EXPR: long comparison */
            mask_le[i] = (p1l[i] <= p2l[i]) ? 1 : 0;
            
            /* Also use conditional select operations */
            select_gt[i] = (p1c[i] > p2c[i]) ? p1c[i] : p2c[i];
            select_ge[i] = (p1s[i] >= p2s[i]) ? p1s[i] : p2s[i];
            select_lt[i] = (p1i[i] < p2i[i]) ? p1i[i] : p2i[i];
            select_le[i] = (p1l[i] <= p2l[i]) ? p1l[i] : p2l[i];
        }
        
        /* Second inner loop with non-constant stride */
        /* This creates complex memory access patterns */
        for (int i = 0; i < N/2; i++) {
            int idx = i * 2;  /* Compile-time constant stride */
            
            /* Mixed comparisons with stride */
            mask_gt[idx] |= (p1i[idx] > p2i[idx]) ? 1 : 0;
            mask_ge[idx] |= (p1i[idx] >= p2i[idx]) ? 1 : 0;
            mask_lt[idx] |= (p1i[idx] < p2i[idx]) ? 1 : 0;
            mask_le[idx] |= (p1i[idx] <= p2i[idx]) ? 1 : 0;
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt[i] + mask_ge[i] + mask_lt[i] + mask_le[i];
            total_checksum += select_gt[i] + select_ge[i] + select_lt[i] + select_le[i];
        }
    }
    
    /* Also test with scalar comparisons */
    int scalar = use_arg;
    for (int i = 0; i < N; i++) {
        /* Comparisons against scalar */
        mask_gt[i] = (p1i[i] > scalar) ? 1 : 0;
        mask_ge[i] = (p1i[i] >= scalar) ? 1 : 0;
        mask_lt[i] = (p1i[i] < scalar) ? 1 : 0;
        mask_le[i] = (p1i[i] <= scalar) ? 1 : 0;
        
        total_checksum += mask_gt[i] + mask_ge[i] + mask_lt[i] + mask_le[i];
    }
    
    printf("Final checksum: %lld\n", total_checksum);
    return (total_checksum > 0) ? 0 : 1;
}
