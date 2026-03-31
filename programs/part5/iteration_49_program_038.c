#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays with distinct patterns
    int a[N], b[N], c[N];
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 3) % 100;          // Pattern 1: 0, 3, 6, 9, ...
        b[i] = (i * 5) % 100;          // Pattern 2: 0, 5, 10, 15, ...
        c[i] = (i * 7) % 100;          // Pattern 3: 0, 7, 14, 21, ...
    }
    
    // Destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        int cond1 = (a[i] > b[i]) ? 1 : 0;
        // Array element vs constant comparison
        int cond2 = (c[i] > 50) ? 2 : 0;
        // Combine results with observable side-effect
        gt_result[i] = cond1 + cond2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        int cond1 = (a[i] >= b[i]) ? 3 : 0;
        // Array element vs constant comparison
        int cond2 = (c[i] >= 75) ? 4 : 0;
        // Combine results
        ge_result[i] = cond1 + cond2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison (will trigger std::swap)
        int cond1 = (a[i] < b[i]) ? 5 : 0;
        // Array element vs constant comparison
        int cond2 = (c[i] < 25) ? 6 : 0;
        // Combine results
        lt_result[i] = cond1 + cond2;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison (will trigger std::swap)
        int cond1 = (a[i] <= b[i]) ? 7 : 0;
        // Array element vs constant comparison
        int cond2 = (c[i] <= 60) ? 8 : 0;
        // Combine results
        le_result[i] = cond1 + cond2;
    }
    
    // Compute checksum to ensure all loops have observable effects
    unsigned long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    // Additional verification: print first few results
    printf("First 5 results:\n");
    for (int i = 0; i < 5 && i < N; ++i) {
        printf("  i=%d: gt=%d, ge=%d, lt=%d, le=%d\n",
               i, gt_result[i], ge_result[i], lt_result[i], le_result[i]);
    }
    
    return 0;
}
