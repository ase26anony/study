#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define NOINLINE __attribute__((noinline))

/* Worker function with vectorizable comparison loops */
NOINLINE
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize")))
#endif
unsigned int process_comparisons(const short* a, const short* b, int n) {
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
    
    /* Combine results to prevent elimination */
    unsigned int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += (unsigned char)gt_results[i];
        checksum ^= (unsigned char)ge_results[i];
        checksum += (unsigned char)lt_results[i] * 3;
        checksum ^= (unsigned char)le_results[i] * 7;
    }
    
    return checksum;
}

/* Initialize arrays with non-constant pattern */
void init_arrays(short* a, short* b, int n, unsigned int seed) {
    /* Simple LCG to generate varying but deterministic values */
    unsigned int state = seed;
    for (int i = 0; i < n; i++) {
        state = state * 1103515245 + 12345;
        a[i] = (short)((state >> 16) & 0x7FFF);
        
        state = state * 1103515245 + 12345;
        b[i] = (short)((state >> 16) & 0x7FFF);
        
        /* Ensure some differences for comparisons */
        if (i % 7 == 0) {
            b[i] = a[i] + (i % 13);
        } else if (i % 5 == 0) {
            a[i] = b[i] - (i % 11);
        }
    }
}

int main(int argc, char** argv) {
    short array_a[N];
    short array_b[N];
    
    /* Use argv for seed variation */
    unsigned int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    unsigned int total = 0;
    
    /* Multiple calls with different data patterns */
    for (int iter = 0; iter < 3; iter++) {
        init_arrays(array_a, array_b, N, seed + iter * 1000);
        
        /* Process and accumulate results */
        unsigned int result = process_comparisons(array_a, array_b, N);
        total += result;
        
        /* Also test with swapped arrays */
        unsigned int swapped_result = process_comparisons(array_b, array_a, N);
        total ^= swapped_result;
    }
    
    printf("Result checksum: %u\n", total);
    return total != 0 ? 0 : 1;
}
