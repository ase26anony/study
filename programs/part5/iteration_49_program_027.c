#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays with distinct patterns
    int a[N], b[N], c[N];
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 3) % 100;          // Pattern 1
        b[i] = (i * 5 + 7) % 100;      // Pattern 2
        c[i] = (i * 2 + 11) % 100;     // Pattern 3
    }
    
    // Destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        int cond1 = (a[i] > b[i]);
        // Array-to-constant comparison
        int cond2 = (c[i] > 50);
        // Combine results with bitwise AND (mimics BIT_AND_EXPR after transformation)
        gt_result[i] = cond1 & cond2 ? 1 : 0;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        int cond1 = (a[i] >= c[i]);
        // Array-to-constant comparison
        int cond2 = (b[i] >= 25);
        // Combine results with bitwise OR (mimics BIT_IOR_EXPR after transformation)
        ge_result[i] = cond1 | cond2 ? 1 : 0;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Note: The uncovered code swaps cond_expr0 and cond_expr1 for LT_EXPR
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison (swapped operands compared to GT_EXPR)
        int cond1 = (b[i] < a[i]);  // Equivalent to a[i] > b[i] but triggers LT_EXPR
        // Array-to-constant comparison
        int cond2 = (c[i] < 75);
        // Combine results with bitwise AND
        lt_result[i] = cond1 & cond2 ? 1 : 0;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison (swapped operands compared to GE_EXPR)
        int cond1 = (c[i] <= a[i]);  // Equivalent to a[i] >= c[i] but triggers LE_EXPR
        // Array-to-constant comparison
        int cond2 = (b[i] <= 60);
        // Combine results with bitwise OR
        le_result[i] = cond1 | cond2 ? 1 : 0;
    }
    
    // Compute checksum to ensure all loops have observable effects
    unsigned long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    // Additional verification: print first few results
    printf("First 10 results:\n");
    for (int i = 0; i < 10 && i < N; ++i) {
        printf("i=%d: gt=%d, ge=%d, lt=%d, le=%d\n",
               i, gt_result[i], ge_result[i], lt_result[i], le_result[i]);
    }
    
    return 0;
}
