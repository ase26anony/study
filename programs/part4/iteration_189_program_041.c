#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024

/* Worker function with vectorizable comparison loops */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize")))
#endif
static int process_comparisons(const short *a, const short *b, int n) {
    /* Destination arrays for comparison results */
    char gt_res[N], ge_res[N], lt_res[N], le_res[N];
    int i;
    
    /* Loop 1: GT_EXPR (>) */
    for (i = 0; i < n; i++) {
        gt_res[i] = a[i] > b[i];
    }
    
    /* Loop 2: GE_EXPR (>=) */
    for (i = 0; i < n; i++) {
        ge_res[i] = a[i] >= b[i];
    }
    
    /* Loop 3: LT_EXPR (<) */
    for (i = 0; i < n; i++) {
        lt_res[i] = a[i] < b[i];
    }
    
    /* Loop 4: LE_EXPR (<=) */
    for (i = 0; i < n; i++) {
        le_res[i] = a[i] <= b[i];
    }
    
    /* Combine results to prevent dead code elimination */
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum ^= gt_res[i];
        checksum += ge_res[i];
        checksum ^= lt_res[i];
        checksum += le_res[i];
    }
    
    return checksum;
}

/* Initialize arrays with non-constant values */
static void init_arrays(short *a, short *b, int n, int seed) {
    for (int i = 0; i < n; i++) {
        /* Simple LCG to generate non-constant values */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        a[i] = (short)(seed % 1000);
        
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        b[i] = (short)((seed % 1000) + (i % 10));
    }
}

int main(int argc, char *argv[]) {
    short array1[N], array2[N];
    int total = 0;
    
    /* Use command-line arguments to vary initialization */
    int base_seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Multiple calls with different data to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        init_arrays(array1, array2, N, base_seed + iter * 100);
        
        /* Call worker function - result must be used */
        int result = process_comparisons(array1, array2, N);
        total += result;
        
        /* Prevent optimization of intermediate results */
        volatile int dummy = result;
        (void)dummy;
    }
    
    printf("Final checksum: %d\n", total);
    return total != 0 ? 0 : 1;
}
