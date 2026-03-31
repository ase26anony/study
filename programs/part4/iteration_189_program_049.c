#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ITERATIONS 4

/* Worker function with explicit optimization attribute */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize")))
#endif
static int process_comparisons(const short *a, const short *b, int n) {
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
        checksum ^= gt_results[i];
        checksum ^= ge_results[i] << 1;
        checksum ^= lt_results[i] << 2;
        checksum ^= le_results[i] << 3;
    }
    
    return checksum;
}

/* Initialize arrays with non-constant values */
static void init_arrays(short *a, short *b, int n, int seed) {
    for (int i = 0; i < n; i++) {
        /* Simple LCG to generate varying values */
        seed = (seed * 1103515245 + 12345) & 0x7fff;
        a[i] = (short)(seed % 1000);
        
        seed = (seed * 1103515245 + 12345) & 0x7fff;
        b[i] = (short)((seed % 1000) + (i % 3) - 1); /* Slightly different pattern */
    }
}

int main(int argc, char *argv[]) {
    short array_a[N];
    short array_b[N];
    
    int total_result = 0;
    
    /* Process multiple iterations with different data */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Initialize with different seeds each iteration */
        init_arrays(array_a, array_b, N, iter + (argc > 1 ? atoi(argv[1]) : 42));
        
        /* Call worker function - result used to prevent elimination */
        int result = process_comparisons(array_a, array_b, N);
        total_result ^= result;
        
        /* Use volatile to ensure calls aren't optimized away */
        volatile int dummy = result;
        (void)dummy;
    }
    
    printf("Final result: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
