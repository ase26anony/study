#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024

#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize")))
#endif
unsigned int process_comparisons(const short* a, const short* b, int n) {
    unsigned char gt_results[N];
    unsigned char ge_results[N];
    unsigned char lt_results[N];
    unsigned char le_results[N];
    
    unsigned int checksum = 0;
    
    // Loop 1: GT_EXPR (>)
    for (int i = 0; i < n; i++) {
        gt_results[i] = (a[i] > b[i]) ? 1 : 0;
    }
    
    // Loop 2: GE_EXPR (>=)
    for (int i = 0; i < n; i++) {
        ge_results[i] = (a[i] >= b[i]) ? 1 : 0;
    }
    
    // Loop 3: LT_EXPR (<)
    for (int i = 0; i < n; i++) {
        lt_results[i] = (a[i] < b[i]) ? 1 : 0;
    }
    
    // Loop 4: LE_EXPR (<=)
    for (int i = 0; i < n; i++) {
        le_results[i] = (a[i] <= b[i]) ? 1 : 0;
    }
    
    // Combine results to prevent dead code elimination
    for (int i = 0; i < n; i++) {
        checksum += gt_results[i] + ge_results[i] + lt_results[i] + le_results[i];
    }
    
    return checksum;
}

void fill_arrays(short* a, short* b, int n, unsigned int seed) {
    // Simple LCG to generate non-constant values
    unsigned int state = seed;
    for (int i = 0; i < n; i++) {
        state = state * 1103515245 + 12345;
        a[i] = (short)((state >> 16) & 0x7FFF);
        
        state = state * 1103515245 + 12345;
        b[i] = (short)((state >> 16) & 0x7FFF);
    }
}

int main(int argc, char* argv[]) {
    short array_a[N];
    short array_b[N];
    
    // Use command-line argument or time as seed for variability
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    fill_arrays(array_a, array_b, N, seed);
    
    // Call multiple times to ensure execution
    unsigned int total = 0;
    for (int iter = 0; iter < 3; iter++) {
        total += process_comparisons(array_a, array_b, N);
        
        // Modify arrays slightly for next iteration
        for (int i = 0; i < N; i++) {
            array_a[i] += 1;
            array_b[i] += 2;
        }
    }
    
    printf("Result checksum: %u\n", total);
    return total > 0 ? 0 : 1;
}
