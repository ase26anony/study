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
    
    /* Combine results to prevent dead code elimination */
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum ^= gt_results[i];
        checksum += ge_results[i];
        checksum ^= lt_results[i] * 3;
        checksum += le_results[i] * 7;
    }
    
    return checksum;
}

/* Simple pseudo-random generator to avoid constant propagation */
static void init_arrays(short *a, short *b, int n, int seed) {
    unsigned int state = seed;
    for (int i = 0; i < n; i++) {
        /* LCG to generate non-constant values */
        state = state * 1103515245 + 12345;
        a[i] = (short)((state >> 16) & 0x7FFF);
        
        state = state * 1103515245 + 12345;
        b[i] = (short)((state >> 16) & 0x7FFF);
    }
}

int main(int argc, char *argv[]) {
    short array1[N];
    short array2[N];
    
    /* Use command line argument as seed if provided */
    int seed = SEED;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with non-constant data */
    init_arrays(array1, array2, N, seed);
    
    /* Process comparisons multiple times to ensure execution */
    int total_result = 0;
    for (int iter = 0; iter < 3; iter++) {
        /* Modify arrays slightly each iteration */
        for (int i = 0; i < N; i++) {
            array1[i] += iter;
            array2[i] -= iter;
        }
        
        total_result += process_comparisons(array1, array2, N);
    }
    
    /* Use the result to prevent optimization */
    volatile int final_result = total_result;
    printf("Result: %d\n", final_result);
    
    return 0;
}
