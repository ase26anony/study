#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024

/* Worker function with optimization attribute to encourage vectorization */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize")))
#endif
static int process_comparisons(const short *a, const short *b, int n) {
    /* Destination arrays for comparison results */
    char gt_results[N];
    char ge_results[N];
    char lt_results[N];
    char le_results[N];
    
    /* Separate loops for each comparison operator */
    
    /* GT_EXPR case */
    for (int i = 0; i < n; i++) {
        gt_results[i] = a[i] > b[i];
    }
    
    /* GE_EXPR case */
    for (int i = 0; i < n; i++) {
        ge_results[i] = a[i] >= b[i];
    }
    
    /* LT_EXPR case */
    for (int i = 0; i < n; i++) {
        lt_results[i] = a[i] < b[i];
    }
    
    /* LE_EXPR case */
    for (int i = 0; i < n; i++) {
        le_results[i] = a[i] <= b[i];
    }
    
    /* Combine results to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum ^= gt_results[i];
        checksum ^= ge_results[i] << 1;
        checksum ^= lt_results[i] << 2;
        checksum ^= le_results[i] << 3;
    }
    
    return checksum;
}

/* Alternative version with float comparisons */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize", "fast-math")))
#endif
static int process_float_comparisons(const float *a, const float *b, int n) {
    char gt_results[N];
    char ge_results[N];
    char lt_results[N];
    char le_results[N];
    
    /* GT_EXPR case - float */
    for (int i = 0; i < n; i++) {
        gt_results[i] = a[i] > b[i];
    }
    
    /* GE_EXPR case - float */
    for (int i = 0; i < n; i++) {
        ge_results[i] = a[i] >= b[i];
    }
    
    /* LT_EXPR case - float */
    for (int i = 0; i < n; i++) {
        lt_results[i] = a[i] < b[i];
    }
    
    /* LE_EXPR case - float */
    for (int i = 0; i < n; i++) {
        le_results[i] = a[i] <= b[i];
    }
    
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum ^= gt_results[i];
        checksum ^= ge_results[i] << 1;
        checksum ^= lt_results[i] << 2;
        checksum ^= le_results[i] << 3;
    }
    
    return checksum;
}

/* Initialize arrays with non-constant values */
static void init_arrays(short *a, short *b, float *fa, float *fb, int n, int seed) {
    for (int i = 0; i < n; i++) {
        /* Use simple LCG to generate non-constant values */
        seed = seed * 1103515245 + 12345;
        a[i] = (short)((seed >> 16) & 0x7FFF);
        b[i] = (short)((seed >> 8) & 0x7FFF);
        
        /* Different seed for floats */
        int fseed = seed * 1664525 + 1013904223;
        fa[i] = (float)(fseed % 1000) / 100.0f;
        fb[i] = (float)((fseed >> 8) % 1000) / 100.0f;
    }
}

int main(int argc, char *argv[]) {
    short a[N], b[N];
    float fa[N], fb[N];
    
    /* Use argv for seed variation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Initialize with non-constant data */
    init_arrays(a, b, fa, fb, N, seed);
    
    /* Process comparisons multiple times with different data */
    int total_checksum = 0;
    
    /* Integer comparisons */
    total_checksum ^= process_comparisons(a, b, N);
    
    /* Float comparisons (with -ffast-math) */
    total_checksum ^= process_float_comparisons(fa, fb, N);
    
    /* Process with swapped arrays to get different patterns */
    total_checksum ^= process_comparisons(b, a, N);
    
    /* Process with offset arrays */
    if (N > 1) {
        total_checksum ^= process_comparisons(a + 1, b, N - 1);
    }
    
    /* Output result to prevent optimization */
    printf("Result checksum: %d\n", total_checksum);
    
    return total_checksum != 0 ? 0 : 1;
}
