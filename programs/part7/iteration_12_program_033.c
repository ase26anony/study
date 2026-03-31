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
    volatile int init_val = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Declare arrays with different integer types */
    char src1_char[N] __attribute__((aligned(32)));
    short src2_short[N] __attribute__((aligned(32)));
    int src3_int[N] __attribute__((aligned(32)));
    long src4_long[N] __attribute__((aligned(32)));
    
    /* Destination arrays for comparison results */
    int mask_gt[N] __attribute__((aligned(32)));
    int mask_ge[N] __attribute__((aligned(32)));
    int mask_lt[N] __attribute__((aligned(32)));
    int mask_le[N] __attribute__((aligned(32)));
    
    /* Initialize source arrays with patterned data */
    for (int i = 0; i < N; i++) {
        unsigned int r = simple_rand();
        src1_char[i] = (char)((r % 256) - 128);  /* -128 to 127 */
        src2_short[i] = (short)((r % 65536) - 32768); /* -32768 to 32767 */
        src3_int[i] = (int)(r % 1000) - 500;     /* -500 to 499 */
        src4_long[i] = (long)(r % 2000) - 1000;  /* -1000 to 999 */
    }
    
    /* Provide alignment hints to the compiler */
    char *src1_aligned = __builtin_assume_aligned(src1_char, 32);
    short *src2_aligned = __builtin_assume_aligned(src2_short, 32);
    int *src3_aligned = __builtin_assume_aligned(src3_int, 32);
    long *src4_aligned = __builtin_assume_aligned(src4_long, 32);
    int *mask_gt_aligned = __builtin_assume_aligned(mask_gt, 32);
    int *mask_ge_aligned = __builtin_assume_aligned(mask_ge, 32);
    int *mask_lt_aligned = __builtin_assume_aligned(mask_lt, 32);
    int *mask_le_aligned = __builtin_assume_aligned(mask_le, 32);
    
    /* Outer loop controlled by volatile to prevent unrolling */
    volatile int outer_limit = OUTER_ITER;
    long total_checksum = 0;
    
    for (int outer = 0; outer < outer_limit; outer++) {
        /* Inner loop with all four comparison types */
        for (int i = 0; i < N; i++) {
            /* Access with constant stride (every other element) */
            int idx = (i * 2) % N;
            
            /* GT_EXPR: char comparison */
            mask_gt_aligned[idx] = (src1_aligned[idx] > (char)(init_val + i % 64)) ? 1 : 0;
            
            /* GE_EXPR: short comparison */
            mask_ge_aligned[idx] = (src2_aligned[idx] >= (short)(init_val - i % 32)) ? 1 : 0;
            
            /* LT_EXPR: int comparison */
            mask_lt_aligned[idx] = (src3_aligned[idx] < (int)(init_val * (i % 8 + 1))) ? 1 : 0;
            
            /* LE_EXPR: long comparison */
            mask_le_aligned[idx] = (src4_aligned[idx] <= (long)(init_val + outer * 10)) ? 1 : 0;
            
            /* Additional comparisons with array-to-array operations */
            if (i > 0) {
                /* Mixed comparisons to trigger different code paths */
                mask_gt_aligned[idx] |= (src1_aligned[idx] > src1_aligned[(idx + 1) % N]) ? 2 : 0;
                mask_ge_aligned[idx] |= (src2_aligned[idx] >= src2_aligned[(idx + 2) % N]) ? 2 : 0;
                mask_lt_aligned[idx] |= (src3_aligned[idx] < src3_aligned[(idx + 3) % N]) ? 2 : 0;
                mask_le_aligned[idx] |= (src4_aligned[idx] <= src4_aligned[(idx + 4) % N]) ? 2 : 0;
            }
        }
        
        /* Consume results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += mask_gt[i] + mask_ge[i] * 3 + mask_lt[i] * 5 + mask_le[i] * 7;
        }
        
        /* Modify source data slightly for next outer iteration */
        for (int i = 0; i < N; i += 4) {
            src1_char[i] += (char)(outer % 4);
            src2_short[i] += (short)(outer % 8);
            src3_int[i] += outer;
            src4_long[i] += outer * 2L;
        }
    }
    
    /* Print checksum to ensure side effects are observable */
    printf("Final checksum: %ld\n", total_checksum);
    
    /* Additional test with conditional select operations */
    int select_results[N];
    int *select_aligned = __builtin_assume_aligned(select_results, 32);
    
    for (int i = 0; i < N; i++) {
        /* Use comparisons in conditional select operations */
        select_aligned[i] = (src3_int[i] > src3_int[(i + 1) % N]) ? 
                           src3_int[i] : src3_int[(i + 1) % N];
        select_aligned[i] = (src2_short[i] >= (short)init_val) ? 
                           select_aligned[i] + 1 : select_aligned[i] - 1;
        select_aligned[i] = (src1_char[i] < (char)(init_val / 2)) ? 
                           select_aligned[i] * 2 : select_aligned[i] / 2;
        select_aligned[i] = (src4_long[i] <= (long)(init_val * 2)) ? 
                           select_aligned[i] + 100 : select_aligned[i] - 100;
    }
    
    /* Consume select results */
    int select_sum = 0;
    for (int i = 0; i < N; i++) {
        select_sum += select_results[i];
    }
    printf("Select sum: %d\n", select_sum);
    
    return (total_checksum != 0 || select_sum != 0) ? 0 : 1;
}
