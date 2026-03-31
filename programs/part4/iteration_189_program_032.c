/* Test program to cover vector comparison lowering in tree-vect-stmts.cc
 * Lines 12216-12233: GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR lowering to bitwise ops
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024  /* Multiple of typical vector width (128/256/512 bits) */

#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize")))
#endif
static unsigned int process_comparisons(const short* a, const short* b, int n) {
    /* Destination arrays for comparison results */
    char gt_res[N], ge_res[N], lt_res[N], le_res[N];
    unsigned int checksum = 0;
    
    /* Loop 1: GT_EXPR (>) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR */
    for (int i = 0; i < n; i++) {
        gt_res[i] = (a[i] > b[i]);
    }
    
    /* Loop 2: GE_EXPR (>=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR */
    for (int i = 0; i < n; i++) {
        ge_res[i] = (a[i] >= b[i]);
    }
    
    /* Loop 3: LT_EXPR (<) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR with swap */
    for (int i = 0; i < n; i++) {
        lt_res[i] = (a[i] < b[i]);
    }
    
    /* Loop 4: LE_EXPR (<=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
    for (int i = 0; i < n; i++) {
        le_res[i] = (a[i] <= b[i]);
    }
    
    /* Combine results to prevent elimination and create checksum */
    for (int i = 0; i < n; i++) {
        checksum += (unsigned int)gt_res[i] + 
                   (unsigned int)ge_res[i] * 3 + 
                   (unsigned int)lt_res[i] * 7 + 
                   (unsigned int)le_res[i] * 11;
    }
    
    return checksum;
}

/* Alternative version with floating-point comparisons */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize", "fast-math")))
#endif
static unsigned int process_fp_comparisons(const float* a, const float* b, int n) {
    int gt_res[N], ge_res[N], lt_res[N], le_res[N];
    unsigned int checksum = 0;
    
    /* Separate loops to ensure each comparison type gets its own vectorization */
    for (int i = 0; i < n; i++) {
        gt_res[i] = (a[i] > b[i]);
    }
    
    for (int i = 0; i < n; i++) {
        ge_res[i] = (a[i] >= b[i]);
    }
    
    for (int i = 0; i < n; i++) {
        lt_res[i] = (a[i] < b[i]);
    }
    
    for (int i = 0; i < n; i++) {
        le_res[i] = (a[i] <= b[i]);
    }
    
    for (int i = 0; i < n; i++) {
        checksum += (unsigned int)gt_res[i] ^ 
                   (unsigned int)ge_res[i] ^ 
                   (unsigned int)lt_res[i] ^ 
                   (unsigned int)le_res[i];
    }
    
    return checksum;
}

/* Initialize arrays with non-constant pattern */
static void init_arrays(short* a, short* b, float* fa, float* fb, int seed) {
    for (int i = 0; i < N; i++) {
        /* Use seed to create varying but deterministic patterns */
        a[i] = (short)((i * 1789 + seed * 37) % 32767);
        b[i] = (short)((i * 1787 + seed * 41) % 32767);
        fa[i] = (float)((i * 1789 + seed * 37) % 1000) / 3.14159f;
        fb[i] = (float)((i * 1787 + seed * 41) % 1000) / 2.71828f;
    }
}

int main(int argc, char** argv) {
    short array_a[N], array_b[N];
    float farray_a[N], farray_b[N];
    unsigned int total_checksum = 0;
    
    /* Use argv to create varying seeds */
    int base_seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Multiple iterations to ensure code is executed */
    for (int iter = 0; iter < 3; iter++) {
        int seed = base_seed + iter * 100;
        
        /* Initialize with non-constant data */
        init_arrays(array_a, array_b, farray_a, farray_b, seed);
        
        /* Process integer comparisons */
        unsigned int int_result = process_comparisons(array_a, array_b, N);
        total_checksum ^= int_result;
        
        /* Process floating-point comparisons (with -ffast-math) */
        unsigned int fp_result = process_fp_comparisons(farray_a, farray_b, N);
        total_checksum ^= fp_result;
        
        /* Small perturbation to arrays */
        for (int i = 0; i < N; i += 7) {
            array_a[i] += 1;
            farray_a[i] += 0.5f;
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %u\n", total_checksum);
    
    /* Additional volatile store to ensure all computations complete */
    volatile unsigned int sink = total_checksum;
    (void)sink;
    
    return (total_checksum == 0) ? 0 : 1;
}
