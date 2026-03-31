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

/* Simple pseudo-random generator to avoid constant propagation */
static unsigned simple_rand(unsigned *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(int argc, char *argv[]) {
    short array1[N];
    short array2[N];
    unsigned seed = SEED;
    int i;
    
    /* Initialize with non-constant values */
    for (i = 0; i < N; i++) {
        array1[i] = (short)(simple_rand(&seed) % 1000);
        array2[i] = (short)(simple_rand(&seed) % 1000);
    }
    
    /* Process multiple times to ensure execution */
    int total = 0;
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    
    for (i = 0; i < iterations; i++) {
        /* Modify arrays slightly each iteration */
        array1[i % N] += i;
        array2[i % N] -= i;
        
        total += process_comparisons(array1, array2, N);
    }
    
    printf("Result checksum: %d\n", total);
    return total != 0 ? 0 : 1;
}
