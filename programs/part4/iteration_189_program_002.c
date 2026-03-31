/* Vector comparison test to cover GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR lowering */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024  /* Multiple of typical vector width (128/256/512 bits) */

/* Worker function with vectorizable comparison loops */
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
    
    /* Loop 1: GT_EXPR (>) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR */
    for (i = 0; i < n; i++) {
        gt_results[i] = (a[i] > b[i]);
    }
    
    /* Loop 2: GE_EXPR (>=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR */
    for (i = 0; i < n; i++) {
        ge_results[i] = (a[i] >= b[i]);
    }
    
    /* Loop 3: LT_EXPR (<) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR with swap */
    for (i = 0; i < n; i++) {
        lt_results[i] = (a[i] < b[i]);
    }
    
    /* Loop 4: LE_EXPR (<=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
    for (i = 0; i < n; i++) {
        le_results[i] = (a[i] <= b[i]);
    }
    
    /* Combine results to prevent dead code elimination */
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += gt_results[i] + ge_results[i] + lt_results[i] + le_results[i];
    }
    
    return checksum;
}

/* Alternative version with floating-point comparisons */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize", "fast-math")))
#endif
static int process_fp_comparisons(const float* a, const float* b, int n) {
    char gt_results[N];
    char ge_results[N];
    char lt_results[N];
    char le_results[N];
    
    int i;
    
    /* Separate loops to ensure each comparison type is vectorized independently */
    for (i = 0; i < n; i++) {
        gt_results[i] = (a[i] > b[i]);
    }
    
    for (i = 0; i < n; i++) {
        ge_results[i] = (a[i] >= b[i]);
    }
    
    for (i = 0; i < n; i++) {
        lt_results[i] = (a[i] < b[i]);
    }
    
    for (i = 0; i < n; i++) {
        le_results[i] = (a[i] <= b[i]);
    }
    
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += gt_results[i] + ge_results[i] + lt_results[i] + le_results[i];
    }
    
    return checksum;
}

/* Initialize arrays with non-constant patterns */
static void init_arrays(short* a, short* b, int n, unsigned int seed) {
    unsigned int r = seed;
    for (int i = 0; i < n; i++) {
        /* Simple LCG to generate varying values */
        r = r * 1103515245 + 12345;
        a[i] = (short)(r & 0x7FFF);
        
        r = r * 1103515245 + 12345;
        b[i] = (short)((r & 0x7FFF) - 1000);  /* Ensure some differences */
    }
}

static void init_fp_arrays(float* a, float* b, int n, unsigned int seed) {
    unsigned int r = seed;
    for (int i = 0; i < n; i++) {
        r = r * 1103515245 + 12345;
        a[i] = (float)(r % 1000) / 10.0f;
        
        r = r * 1103515245 + 12345;
        b[i] = (float)(r % 1000) / 10.0f;
    }
}

int main(int argc, char* argv[]) {
    short a_short[N];
    short b_short[N];
    float a_float[N];
    float b_float[N];
    
    /* Use argv for seed variation */
    unsigned int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize arrays */
    init_arrays(a_short, b_short, N, seed);
    init_fp_arrays(a_float, b_float, N, seed + 1);
    
    /* Process comparisons multiple times with different data */
    int total_checksum = 0;
    
    /* Integer comparisons */
    total_checksum += process_comparisons(a_short, b_short, N);
    
    /* Shuffle data slightly */
    for (int i = 0; i < N/2; i++) {
        short tmp = a_short[i];
        a_short[i] = b_short[i];
        b_short[i] = tmp;
    }
    
    total_checksum += process_comparisons(a_short, b_short, N);
    
    /* Floating-point comparisons */
    total_checksum += process_fp_comparisons(a_float, b_float, N);
    
    /* Shuffle FP data */
    for (int i = 0; i < N/2; i++) {
        float tmp = a_float[i];
        a_float[i] = b_float[i];
        b_float[i] = tmp;
    }
    
    total_checksum += process_fp_comparisons(a_float, b_float, N);
    
    /* Use result to prevent optimization */
    printf("Total checksum: %d\n", total_checksum);
    
    /* Additional volatile store to ensure computations aren't eliminated */
    volatile int sink = total_checksum;
    (void)sink;
    
    return 0;
}
