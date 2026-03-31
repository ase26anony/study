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
    uint32_t seed = (uint32_t)(argc > 1 ? atoi(argv[1]) : 1234);
    
    /* Declare source arrays with different integer types */
    char src1_char[N] __attribute__((aligned(64)));
    short src1_short[N] __attribute__((aligned(64)));
    int src1_int[N] __attribute__((aligned(64)));
    long src1_long[N] __attribute__((aligned(64)));
    
    char src2_char[N] __attribute__((aligned(64)));
    short src2_short[N] __attribute__((aligned(64)));
    int src2_int[N] __attribute__((aligned(64)));
    long src2_long[N] __attribute__((aligned(64)));
    
    /* Destination mask arrays for comparison results */
    int mask_gt[N] __attribute__((aligned(64)));
    int mask_ge[N] __attribute__((aligned(64)));
    int mask_lt[N] __attribute__((aligned(64)));
    int mask_le[N] __attribute__((aligned(64)));
    
    /* Initialize source arrays with patterned data */
    for (int i = 0; i < N; i++) {
        uint32_t r = simple_rand(&seed);
        src1_char[i] = (char)(r & 0xFF);
        src1_short[i] = (short)(r & 0xFFFF);
        src1_int[i] = (int)r;
        src1_long[i] = (long)r;
        
        r = simple_rand(&seed);
        src2_char[i] = (char)(r & 0xFF);
        src2_short[i] = (short)(r & 0xFFFF);
        src2_int[i] = (int)r;
        src2_long[i] = (long)r;
    }
    
    /* Provide alignment hints to the compiler */
    char *p1c = __builtin_assume_aligned(src1_char, 64);
    short *p1s = __builtin_assume_aligned(src1_short, 64);
    int *p1i = __builtin_assume_aligned(src1_int, 64);
    long *p1l = __builtin_assume_aligned(src1_long, 64);
    
    char *p2c = __builtin_assume_aligned(src2_char, 64);
    short *p2s = __builtin_assume_aligned(src2_short, 64);
    int *p2i = __builtin_assume_aligned(src2_int, 64);
    long *p2l = __builtin_assume_aligned(src2_long, 64);
    
    int *pm_gt = __builtin_assume_aligned(mask_gt, 64);
    int *pm_ge = __builtin_assume_aligned(mask_ge, 64);
    int *pm_lt = __builtin_assume_aligned(mask_lt, 64);
    int *pm_le = __builtin_assume_aligned(mask_le, 64);
    
    volatile int outer_bound = OUTER_ITER;
    long long total_checksum = 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Use different data types for different comparisons to stress type conversion */
            
            /* GT_EXPR: > comparison with char type */
            pm_gt[i] = (p1c[i] > p2c[i]) ? 1 : 0;
            
            /* GE_EXPR: >= comparison with short type, using stride */
            int idx = (i * 2) % N;  /* Non-constant stride pattern */
            pm_ge[i] = (p1s[idx] >= p2s[idx]) ? 1 : 0;
            
            /* LT_EXPR: < comparison with int type */
            pm_lt[i] = (p1i[i] < p2i[i]) ? 1 : 0;
            
            /* LE_EXPR: <= comparison with long type, using stride */
            pm_le[i] = (p1l[idx] <= p2l[idx]) ? 1 : 0;
            
            /* Additional comparisons with scalar values */
            pm_gt[i] |= (p1i[i] > 1000) ? 2 : 0;
            pm_ge[i] |= (p1s[i] >= -500) ? 2 : 0;
            pm_lt[i] |= (p1c[i] < 50) ? 2 : 0;
            pm_le[i] |= (p1l[i] <= 1000000) ? 2 : 0;
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += pm_gt[i] + pm_ge[i] + pm_lt[i] + pm_le[i];
        }
        
        /* Modify source data slightly to prevent complete optimization */
        for (int i = 0; i < N; i++) {
            p1c[i] += (char)(i & 0x7F);
            p2c[i] -= (char)(i & 0x3F);
        }
    }
    
    /* Use conditional select operations to force mask generation */
    int select_results[N] __attribute__((aligned(64)));
    int *psel = __builtin_assume_aligned(select_results, 64);
    
    for (int i = 0; i < N; i++) {
        /* Conditional selects using comparison results */
        psel[i] = (p1i[i] > p2i[i]) ? p1i[i] : p2i[i];
        psel[i] += (p1i[i] >= p2i[i]) ? p1i[i] : -p2i[i];
        psel[i] += (p1i[i] < p2i[i]) ? p1i[i] * 2 : p2i[i];
        psel[i] += (p1i[i] <= p2i[i]) ? p1i[i] / 2 : p2i[i];
        
        total_checksum += psel[i];
    }
    
    printf("Final checksum: %lld\n", total_checksum);
    return (total_checksum > 0) ? 0 : 1;
}
