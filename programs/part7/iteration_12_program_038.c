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
    
    uint32_t seed = 42;
    
    /* Initialize source arrays with patterned data */
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
    
    /* Provide alignment hints to the compiler */
    char * __restrict a_char = src1_char;
    char * __restrict b_char = src2_char;
    short * __restrict a_short = src1_short;
    short * __restrict b_short = src2_short;
    int * __restrict a_int = src1_int;
    int * __restrict b_int = src2_int;
    long * __restrict a_long = src1_long;
    long * __restrict b_long = src2_long;
    
    __builtin_assume_aligned(a_char, 32);
    __builtin_assume_aligned(b_char, 32);
    __builtin_assume_aligned(a_short, 32);
    __builtin_assume_aligned(b_short, 32);
    __builtin_assume_aligned(a_int, 32);
    __builtin_assume_aligned(b_int, 32);
    __builtin_assume_aligned(a_long, 32);
    __builtin_assume_aligned(b_long, 32);
    
    volatile int outer_bound = OUTER_ITER;
    long long total_checksum = 0;
    
    /* Outer loop to potentially trigger outer-loop vectorization */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Mixed type comparisons to stress type conversion logic */
            
            /* GT_EXPR: > comparison */
            mask_gt[i] = (a_int[i] > b_int[i]) && 
                         (a_short[i * 2 % N] > b_short[i * 2 % N]);  /* Non-constant stride */
            
            /* GE_EXPR: >= comparison */
            mask_ge[i] = (a_long[i] >= b_long[i]) || 
                         (a_char[i * 3 % N] >= (char)(use_arg));  /* Array vs scalar */
            
            /* LT_EXPR: < comparison */
            mask_lt[i] = (a_int[i] < b_int[i]) && 
                         (a_short[(i + 1) % N] < b_short[(i + 1) % N]);
            
            /* LE_EXPR: <= comparison */
            mask_le[i] = (a_long[i] <= b_long[i]) || 
                         (a_char[(i * 2 + 1) % N] <= (char)(use_arg + 1));
            
            /* Conditional select operations using comparison results */
            select_gt[i] = (a_int[i] > b_int[i]) ? a_int[i] : b_int[i];
            select_ge[i] = (a_long[i] >= b_long[i]) ? a_long[i] : b_long[i];
            select_lt[i] = (a_int[i] < b_int[i]) ? a_int[i] : b_int[i];
            select_le[i] = (a_long[i] <= b_long[i]) ? a_long[i] : b_long[i];
            
            /* Additional comparisons with different operand orders */
            if (i % 4 == 0) {
                /* Reverse operand order for some iterations */
                mask_gt[i] |= (b_int[i] > a_int[i]);
                mask_lt[i] |= (b_int[i] < a_int[i]);
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt[i] + mask_ge[i] + mask_lt[i] + mask_le[i];
            total_checksum += select_gt[i] + select_ge[i] + select_lt[i] + select_le[i];
        }
        
        /* Modify source data slightly for next outer iteration */
        for (int i = 0; i < N; i += 4) {
            a_int[i] += outer;
            b_int[i] -= outer;
        }
    }
    
    printf("Final checksum: %lld\n", total_checksum);
    return (int)(total_checksum % 256);
}
