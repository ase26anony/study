/* Vector comparison test to cover GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR lowering */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024  /* Multiple of typical vector width (128/256/512 bits) */

/* Worker function with aggressive optimization */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize")))
#endif
static int process_comparisons(const short *a, const short *b, int n) {
    /* Destination arrays for comparison results */
    char gt_res[N], ge_res[N], lt_res[N], le_res[N];
    int i;
    
    /* Four separate loops to ensure each comparison type is vectorized independently */
    
    /* GT_EXPR: a[i] > b[i] */
    for (i = 0; i < n; i++) {
        gt_res[i] = a[i] > b[i];
    }
    
    /* GE_EXPR: a[i] >= b[i] */
    for (i = 0; i < n; i++) {
        ge_res[i] = a[i] >= b[i];
    }
    
    /* LT_EXPR: a[i] < b[i] */
    for (i = 0; i < n; i++) {
        lt_res[i] = a[i] < b[i];
    }
    
    /* LE_EXPR: a[i] <= b[i] */
    for (i = 0; i < n; i++) {
        le_res[i] = a[i] <= b[i];
    }
    
    /* Combine results to prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < n; i++) {
        sum += gt_res[i] + ge_res[i] + lt_res[i] + le_res[i];
    }
    
    return sum;
}

/* Alternative with floating-point comparisons */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize", "fast-math")))
#endif
static int process_fp_comparisons(const float *a, const float *b, int n) {
    char gt_res[N], ge_res[N], lt_res[N], le_res[N];
    int i;
    
    /* GT_EXPR with floats */
    for (i = 0; i < n; i++) {
        gt_res[i] = a[i] > b[i];
    }
    
    /* GE_EXPR with floats */
    for (i = 0; i < n; i++) {
        ge_res[i] = a[i] >= b[i];
    }
    
    /* LT_EXPR with floats */
    for (i = 0; i < n; i++) {
        lt_res[i] = a[i] < b[i];
    }
    
    /* LE_EXPR with floats */
    for (i = 0; i < n; i++) {
        le_res[i] = a[i] <= b[i];
    }
    
    int sum = 0;
    for (i = 0; i < n; i++) {
        sum += gt_res[i] + ge_res[i] + lt_res[i] + le_res[i];
    }
    
    return sum;
}

/* Initialize arrays with non-constant pattern */
static void init_arrays(short *a, short *b, float *fa, float *fb, int seed) {
    for (int i = 0; i < N; i++) {
        /* Use seed to create varying but deterministic patterns */
        a[i] = (short)((i * 1789 + seed * 7919) % 32767);
        b[i] = (short)((i * 1787 + seed * 7927) % 32767);
        fa[i] = (float)((i * 1789 + seed * 7919) % 1000) / 3.14159f;
        fb[i] = (float)((i * 1787 + seed * 7927) % 1000) / 2.71828f;
    }
}

int main(int argc, char **argv) {
    short array_a[N], array_b[N];
    float farray_a[N], farray_b[N];
    int total = 0;
    
    /* Use argv to create varying seeds */
    int base_seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Multiple calls with different data to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        init_arrays(array_a, array_b, farray_a, farray_b, base_seed + iter);
        
        /* Process integer comparisons */
        total += process_comparisons(array_a, array_b, N);
        
        /* Process floating-point comparisons */
        total += process_fp_comparisons(farray_a, farray_b, N);
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", total);
    
    /* Additional volatile store to ensure all computations complete */
    volatile int sink = total;
    
    return (total > 0) ? 0 : 1;
}
