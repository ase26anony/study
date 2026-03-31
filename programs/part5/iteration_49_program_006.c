#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays with distinct patterns
    int a[N], b[N], c[N];
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize with reproducible patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;          // Pattern 1: 0, 3, 6, 9, ...
        b[i] = (i * 5) % 100;          // Pattern 2: 0, 5, 10, 15, ...
        c[i] = (i * 7) % 100;          // Pattern 3: 0, 7, 14, 21, ...
    }
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mixes array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; i++) {
        // Array-to-array comparison
        int cond1 = a[i] > b[i];
        // Array-to-constant comparison
        int cond2 = c[i] > 50;
        // Combine results with bitwise operations
        gt_result[i] = (cond1 & cond2) ? 1 : 0;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; i++) {
        // Array-to-array comparison
        int cond1 = a[i] >= b[i];
        // Array-to-constant comparison
        int cond2 = c[i] >= 30;
        // Combine results with bitwise operations
        ge_result[i] = (cond1 | cond2) ? 2 : 0;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Note: The uncovered code swaps cond_expr0 and cond_expr1 for LT_EXPR
    for (int i = 0; i < N; i++) {
        // Array-to-array comparison (swapped operands in the transformation)
        int cond1 = b[i] < a[i];  // Equivalent to a[i] > b[i] but triggers LT_EXPR path
        // Array-to-constant comparison
        int cond2 = 20 < c[i];    // Constant on left side
        // Combine results
        lt_result[i] = (cond1 & cond2) ? 3 : 0;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; i++) {
        // Array-to-array comparison (swapped operands in the transformation)
        int cond1 = b[i] <= a[i];  // Equivalent to a[i] >= b[i] but triggers LE_EXPR path
        // Array-to-constant comparison with constant on left
        int cond2 = 40 <= c[i];
        // Combine results
        le_result[i] = (cond1 | cond2) ? 4 : 0;
    }
    
    // Additional loops to ensure all comparison patterns are covered
    // These use different data types as specified in requirements
    
    // Using unsigned short to test different integer type
    unsigned short us_a[N], us_b[N];
    int us_result[N];
    
    for (int i = 0; i < N; i++) {
        us_a[i] = (i * 11) % 256;
        us_b[i] = (i * 13) % 256;
    }
    
    // Loop 5: Mixed comparisons with unsigned short
    for (int i = 0; i < N; i++) {
        // Test all four operators with unsigned types
        int cond_gt = us_a[i] > us_b[i];
        int cond_ge = us_a[i] >= 100;
        int cond_lt = us_a[i] < us_b[i];
        int cond_le = us_a[i] <= 200;
        
        // Combine all conditions
        us_result[i] = (cond_gt & cond_ge & cond_lt & cond_le) ? 5 : 0;
    }
    
    // Using long to test wider integer type
    long long la[N], lb[N];
    int ll_result[N];
    
    for (int i = 0; i < N; i++) {
        la[i] = (long long)i * 1000;
        lb[i] = (long long)i * 1000 + 500;
    }
    
    // Loop 6: Long integer comparisons
    for (int i = 0; i < N; i++) {
        // Test all four operators with long types
        int cond_gt = la[i] > lb[i];
        int cond_ge = la[i] >= 5000;
        int cond_lt = la[i] < lb[i];
        int cond_le = la[i] <= 10000;
        
        // Combine conditions
        ll_result[i] = (cond_gt | cond_ge | cond_lt | cond_le) ? 6 : 0;
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
        checksum += us_result[i] + ll_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification to ensure all loops are executed
    volatile int dummy = 0;
    dummy += gt_result[0] + ge_result[N-1] + lt_result[N/2] + le_result[N/4];
    dummy += us_result[N/3] + ll_result[N*2/3];
    
    return 0;
}
