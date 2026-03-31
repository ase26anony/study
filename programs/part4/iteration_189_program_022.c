#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define SEED 42

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
    
    /* Loop 1: GT_EXPR (>) */
    for (int i = 0; i < n; i++) {
        gt_results[i] = a[i] > b[i];
    }
    
    /* Loop 2: GE_EXPR (>=) */
    for (int i = 0; i < n; i++) {
        ge_results[i] = a[i] >= b[i];
    }
    
    /* Loop 3: LT_EXPR (<) */
    for (int i = 0; i < n; i++) {
        lt_results[i] = a[i] < b[i];
    }
    
    /* Loop 4: LE_EXPR (<=) */
    for (int i = 0; i < n; i++) {
        le_results[i] = a[i] <= b[i];
    }
    
    /* Combine results to create data dependency and prevent elimination */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += gt_results[i] + ge_results[i] + lt_results[i] + le_results[i];
    }
    
    return checksum;
}

/* Initialize arrays with non-constant values */
static void init_arrays(short *a, short *b, int n, int seed) {
    /* Simple LCG to generate varying but deterministic values */
    unsigned int state = seed;
    for (int i = 0; i < n; i++) {
        state = state * 1103515245 + 12345;
        a[i] = (short)(state >> 16) & 0x7FFF;
        
        state = state * 1103515245 + 12345;
        b[i] = (short)(state >> 16) & 0x7FFF;
        
        /* Ensure some differences for comparisons */
        if (i % 3 == 0) b[i] = a[i] + 1;
        if (i % 5 == 0) a[i] = b[i] + 1;
        if (i % 7 == 0) a[i] = b[i];
    }
}

int main(int argc, char *argv[]) {
    short array_a[N];
    short array_b[N];
    
    /* Use command-line argument as seed if provided */
    int seed = SEED;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with non-constant data */
    init_arrays(array_a, array_b, N, seed);
    
    /* Process comparisons multiple times to ensure execution */
    int total_checksum = 0;
    for (int iter = 0; iter < 3; iter++) {
        /* Modify arrays slightly each iteration */
        for (int i = 0; i < N; i++) {
            array_a[i] += iter;
            array_b[i] -= iter;
        }
        
        total_checksum += process_comparisons(array_a, array_b, N);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result checksum: %d\n", total_checksum);
    
    return total_checksum != 0 ? 0 : 1;
}
