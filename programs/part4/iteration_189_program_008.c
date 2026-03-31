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
    
    /* Loop 1: GT_EXPR (>) */
    for (i = 0; i < n; i++) {
        gt_results[i] = (a[i] > b[i]);
    }
    
    /* Loop 2: GE_EXPR (>=) */
    for (i = 0; i < n; i++) {
        ge_results[i] = (a[i] >= b[i]);
    }
    
    /* Loop 3: LT_EXPR (<) */
    for (i = 0; i < n; i++) {
        lt_results[i] = (a[i] < b[i]);
    }
    
    /* Loop 4: LE_EXPR (<=) */
    for (i = 0; i < n; i++) {
        le_results[i] = (a[i] <= b[i]);
    }
    
    /* Combine results to prevent elimination */
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += gt_results[i] + ge_results[i] + lt_results[i] + le_results[i];
    }
    
    return checksum;
}

/* Initialize arrays with non-constant patterns */
static void init_arrays(short* a, short* b, int seed) {
    for (int i = 0; i < N; i++) {
        /* Use seed to create varying but deterministic patterns */
        a[i] = (short)((i * 1789 + seed * 1931) % 32767);
        b[i] = (short)((i * 2017 + seed * 1663) % 32767);
        
        /* Ensure some elements satisfy different comparison conditions */
        if (i % 7 == 0) a[i] = b[i];           /* Equal case */
        if (i % 11 == 0) a[i] = b[i] + 100;    /* Definitely greater */
        if (i % 13 == 0) a[i] = b[i] - 100;    /* Definitely less */
    }
}

int main(int argc, char** argv) {
    short array_a[N];
    short array_b[N];
    
    /* Use argv to create varying seeds */
    int base_seed = 0;
    if (argc > 1) {
        base_seed = atoi(argv[1]);
    }
    
    int total_checksum = 0;
    
    /* Multiple calls with different data patterns */
    for (int iter = 0; iter < 3; iter++) {
        init_arrays(array_a, array_b, base_seed + iter * 1000);
        
        /* Process comparisons - this should trigger vectorization */
        int result = process_comparisons(array_a, array_b, N);
        total_checksum ^= result;  /* Combine results */
        
        /* Use volatile to ensure calls aren't eliminated */
        volatile int dummy = result;
        (void)dummy;
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return total_checksum != 0 ? 0 : 1;
}
