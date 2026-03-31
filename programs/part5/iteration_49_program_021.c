#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Initialize source arrays with distinct patterns
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Initialize with reproducible pseudo-random values
    srand(42);
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = i % 100;
        d[i] = (unsigned short)(rand() % 65535);
        e[i] = (long)(rand() % 10000);
    }
    
    // Destination arrays for results
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        int cmp1 = (a[i] > b[i]) ? 1 : 0;
        // Array-to-constant comparison
        int cmp2 = (a[i] > 500) ? 2 : 0;
        // Combine results with side-effect
        gt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison with different types
        int cmp1 = (c[i] >= d[i]) ? 3 : 0;
        // Array-to-constant comparison
        int cmp2 = (b[i] >= 250) ? 4 : 0;
        // Store result to prevent dead code elimination
        ge_result[i] = cmp1 + cmp2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        int cmp1 = (a[i] < b[i]) ? 5 : 0;
        // Array-to-constant comparison with different type
        int cmp2 = (e[i] < 5000L) ? 6 : 0;
        // Store combined result
        lt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Multiple comparisons to increase coverage
        int cmp1 = (c[i] <= d[i]) ? 7 : 0;
        int cmp2 = (a[i] <= 750) ? 8 : 0;
        int cmp3 = (e[i] <= 7500L) ? 9 : 0;
        // Store result with observable side-effect
        le_result[i] = cmp1 + cmp2 + cmp3;
    }
    
    // Additional loops with different integer types to ensure coverage
    // Loop 5: Mixed comparisons in same loop
    int mixed_result[N];
    for (int i = 0; i < N; ++i) {
        int val = 0;
        val += (a[i] > b[i]) ? 10 : 0;   // GT_EXPR
        val += (c[i] >= i) ? 20 : 0;     // GE_EXPR  
        val += (d[i] < 32768) ? 30 : 0;  // LT_EXPR
        val += (e[i] <= 8000L) ? 40 : 0; // LE_EXPR
        mixed_result[i] = val;
    }
    
    // Compute checksum to prevent optimization and verify results
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += mixed_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification print to ensure all loops execute
    printf("Sample results:\n");
    printf("gt_result[0]=%d, ge_result[100]=%d\n", gt_result[0], ge_result[100]);
    printf("lt_result[200]=%d, le_result[300]=%d\n", lt_result[200], le_result[300]);
    printf("mixed_result[400]=%d\n", mixed_result[400]);
    
    return 0;
}
