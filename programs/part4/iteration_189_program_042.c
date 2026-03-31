/* Vector comparison test to cover GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR lowering */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024  /* Multiple of typical vector width (128/256/512 bits) */

/* Worker function with explicit optimization attribute */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize")))
#endif
static int process_comparisons(const short* a, const short* b, int n) {
    /* Destination arrays for comparison results */
    char gt_results[N];
    char ge_results[N];
    char lt_results[N];
    char le_results[N];
    
    int i;
    
    /* Loop 1: GT_EXPR (>) */
    for (i = 0; i < n; i++) {
        gt_results[i] = a[i] > b[i];
    }
    
    /* Loop 2: GE_EXPR (>=) */
    for (i = 0; i < n; i++) {
        ge_results[i] = a[i] >= b[i];
    }
    
    /* Loop 3: LT_EXPR (<) */
    for (i = 0; i < n; i++) {
        lt_results[i] = a[i] < b[i];
    }
    
    /* Loop 4: LE_EXPR (<=) */
    for (i = 0; i < n; i++) {
        le_results[i] = a[i] <= b[i];
    }
    
    /* Combine results to prevent elimination */
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += gt_results[i] + ge_results[i] + lt_results[i] + le_results[i];
    }
    
    return checksum;
}

/* Alternative with floating point comparisons */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize", "fast-math")))
#endif
static int process_fp_comparisons(const float* a, const float* b, int n) {
    char gt_results[N];
    char ge_results[N];
    char lt_results[N];
    char le_results[N];
    
    int i;
    
    /* Separate loops for each comparison type */
    for (i = 0; i < n; i++) {
        gt_results[i] = a[i] > b[i];
    }
    
    for (i = 0; i < n; i++) {
        ge_results[i] = a[i] >= b[i];
    }
    
    for (i = 0; i < n; i++) {
        lt_results[i] = a[i] < b[i];
    }
    
    for (i = 0; i < n; i++) {
        le_results[i] = a[i] <= b[i];
    }
    
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += gt_results[i] + ge_results[i] + lt_results[i] + le_results[i];
    }
    
    return checksum;
}

/* Simple pseudo-random generator to avoid constant propagation */
static unsigned int simple_rand(unsigned int seed) {
    return seed * 1103515245 + 12345;
}

int main(int argc, char* argv[]) {
    short a_short[N], b_short[N];
    float a_float[N], b_float[N];
    
    /* Initialize with non-constant values */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    for (int i = 0; i < N; i++) {
        seed = simple_rand(seed);
        a_short[i] = (short)(seed % 1000);
        seed = simple_rand(seed);
        b_short[i] = (short)(seed % 1000);
        
        seed = simple_rand(seed);
        a_float[i] = (float)(seed % 1000) * 0.1f;
        seed = simple_rand(seed);
        b_float[i] = (float)(seed % 1000) * 0.1f;
    }
    
    /* Process integer comparisons */
    int result1 = process_comparisons(a_short, b_short, N);
    
    /* Process floating-point comparisons */
    int result2 = process_fp_comparisons(a_float, b_float, N);
    
    /* Use results to prevent elimination */
    volatile int final_result = result1 + result2;
    printf("Result: %d\n", final_result);
    
    return 0;
}
