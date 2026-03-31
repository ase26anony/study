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
    
    /* Initialize arrays with pattern to avoid compile-time propagation */
    for (int i = 0; i < N; i++) {
        src1_char[i] = (char)(simple_rand(&seed) % 256 - 128);
        src2_short[i] = (short)(simple_rand(&seed) % 65536 - 32768);
        src3_int[i] = (int)(simple_rand(&seed) % 1000 - 500);
        src4_long[i] = (long)(simple_rand(&seed) % 2000 - 1000);
        src_strided[i*2] = (int)(simple_rand(&seed) % 1000);
    }
    
    /* Use argv to prevent compile-time optimization */
    int offset = (argc > 1) ? atoi(argv[1]) : 0;
    
    /* Outer loop to trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Provide alignment hints */
            char *p1 = __builtin_assume_aligned(src1_char, 32);
            short *p2 = __builtin_assume_aligned(src2_short, 32);
            int *p3 = __builtin_assume_aligned(src3_int, 32);
            long *p4 = __builtin_assume_aligned(src4_long, 32);
            
            /* GT_EXPR: > comparison with char type */
            mask_gt[i] = (p1[i] > (char)(i + offset)) ? 1 : 0;
            
            /* GE_EXPR: >= comparison with short type */
            mask_ge[i] = (p2[i] >= (short)(i * 2 + offset)) ? 1 : 0;
            
            /* LT_EXPR: < comparison with int type */
            mask_lt[i] = (p3[i] < (int)(i * 3 + offset)) ? 1 : 0;
            
            /* LE_EXPR: <= comparison with long type */
            mask_le[i] = (p4[i] <= (long)(i * 4 + offset)) ? 1 : 0;
            
            /* Additional comparisons with mixed types */
            int temp1 = (p1[i] > p2[i]) ? p1[i] : p2[i];
            int temp2 = (p3[i] >= p4[i]) ? p3[i] : p4[i];
            int temp3 = (p1[i] < p3[i]) ? p1[i] : p3[i];
            int temp4 = (p2[i] <= p4[i]) ? p2[i] : p4[i];
            
            /* Use results to prevent elimination */
            mask_gt[i] ^= temp1;
            mask_ge[i] ^= temp2;
            mask_lt[i] ^= temp3;
            mask_le[i] ^= temp4;
        }
        
        /* Second inner loop with constant stride access */
        int *p_strided = __builtin_assume_aligned(src_strided, 32);
        for (int i = 0; i < N; i++) {
            /* Access with stride 2 */
            mask_strided[i] = (p_strided[i*2] > (int)(i + offset)) ? 1 : 0;
            mask_strided[i] |= (p_strided[i*2] >= (int)(i * 2 + offset)) ? 2 : 0;
            mask_strided[i] |= (p_strided[i*2] < (int)(i * 3 + offset)) ? 4 : 0;
            mask_strided[i] |= (p_strided[i*2] <= (int)(i * 4 + offset)) ? 8 : 0;
        }
    }
    
    /* Compute checksum to ensure side effects are observable */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += mask_gt[i] + mask_ge[i] + mask_lt[i] + mask_le[i] + mask_strided[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    return (checksum > 0) ? 0 : 1;
}
