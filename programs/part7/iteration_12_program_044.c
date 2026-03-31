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
    
    /* Destination mask arrays for each comparison type */
    int mask_gt[N] __attribute__((aligned(32)));
    int mask_ge[N] __attribute__((aligned(32)));
    int mask_lt[N] __attribute__((aligned(32)));
    int mask_le[N] __attribute__((aligned(32)));
    
    /* Initialize with patterned data */
    uint32_t seed = 42;
    for (int i = 0; i < N; i++) {
        uint32_t val = simple_rand(&seed);
        src1_char[i] = (char)(val % 256 - 128);
        src1_short[i] = (short)(val % 65536 - 32768);
        src1_int[i] = (int)val;
        src1_long[i] = (long)val * use_arg;
        
        val = simple_rand(&seed);
        src2_char[i] = (char)(val % 256 - 128);
        src2_short[i] = (short)(val % 65536 - 32768);
        src2_int[i] = (int)val;
        src2_long[i] = (long)val * use_arg;
    }
    
    /* Provide alignment hints */
    char *p1c = __builtin_assume_aligned(src1_char, 32);
    short *p1s = __builtin_assume_aligned(src1_short, 32);
    int *p1i = __builtin_assume_aligned(src1_int, 32);
    long *p1l = __builtin_assume_aligned(src1_long, 32);
    
    char *p2c = __builtin_assume_aligned(src2_char, 32);
    short *p2s = __builtin_assume_aligned(src2_short, 32);
    int *p2i = __builtin_assume_aligned(src2_int, 32);
    long *p2l = __builtin_assume_aligned(src2_long, 32);
    
    int *m_gt = __builtin_assume_aligned(mask_gt, 32);
    int *m_ge = __builtin_assume_aligned(mask_ge, 32);
    int *m_lt = __builtin_assume_aligned(mask_lt, 32);
    int *m_le = __builtin_assume_aligned(mask_le, 32);
    
    volatile int outer_bound = OUTER_ITER;
    long checksum = 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride 2 for complex pattern */
            int idx = (i * 2) % N;
            
            /* GT_EXPR: > comparison */
            m_gt[idx] = (p1i[idx] > p2i[idx]) ? 1 : 0;
            
            /* GE_EXPR: >= comparison */
            m_ge[idx] = (p1s[idx] >= p2s[idx]) ? 1 : 0;
            
            /* LT_EXPR: < comparison */
            m_lt[idx] = (p1c[idx] < p2c[idx]) ? 1 : 0;
            
            /* LE_EXPR: <= comparison */
            m_le[idx] = (p1l[idx] <= p2l[idx]) ? 1 : 0;
            
            /* Additional comparisons with scalar */
            int scalar = outer + i % 16;
            m_gt[idx] |= (p1i[idx] > scalar) ? 2 : 0;
            m_ge[idx] |= (p1s[idx] >= scalar) ? 2 : 0;
            m_lt[idx] |= (p1c[idx] < scalar) ? 2 : 0;
            m_le[idx] |= (p1l[idx] <= scalar) ? 2 : 0;
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            checksum += m_gt[i] + m_ge[i] * 3 + m_lt[i] * 5 + m_le[i] * 7;
        }
        
        /* Conditional select operations using comparisons */
        int temp[N] __attribute__((aligned(32)));
        int *ptemp = __builtin_assume_aligned(temp, 32);
        
        for (int i = 0; i < N; i++) {
            /* Use comparisons in conditional select */
            ptemp[i] = (p1i[i] > p2i[i]) ? p1i[i] : p2i[i];
            ptemp[i] = (p1s[i] >= p2s[i]) ? ptemp[i] + p1s[i] : ptemp[i] - p2s[i];
            ptemp[i] = (p1c[i] < p2c[i]) ? ptemp[i] * 2 : ptemp[i] / 2;
            ptemp[i] = (p1l[i] <= p2l[i]) ? ptemp[i] + 1 : ptemp[i] - 1;
            
            checksum += ptemp[i];
        }
    }
    
    printf("Final checksum: %ld\n", checksum);
    return 0;
}
