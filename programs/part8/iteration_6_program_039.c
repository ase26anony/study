#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    int ALIGNED a[N], b[N];
    int ALIGNED gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize arrays with distinct values
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Pattern: 0, 3, 6, 9, ...
        b[i] = (i * 2) % 100;      // Pattern: 0, 2, 4, 6, ...
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) and GE_EXPR (>=) comparisons
    // This should trigger the transformation for greater-than and greater-or-equal
    for (int i = 0; i < N; i++) {
        gt_result[i] = a[i] > b[i];   // GT_EXPR
        ge_result[i] = a[i] >= b[i];  // GE_EXPR
    }
    
    // Loop 2: LT_EXPR (<) and LE_EXPR (<=) comparisons  
    // This should trigger the transformation for less-than and less-or-equal
    for (int i = 0; i < N; i++) {
        lt_result[i] = a[i] < b[i];   // LT_EXPR
        le_result[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Use results to prevent dead code elimination
    // Compute checksum to ensure all comparisons are actually used
    for (int i = 0; i < N; i++) {
        sum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    
    // Print checksum (prevents optimization away)
    printf("Checksum: %d\n", sum);
    
    // Also use in conditional to ensure comparisons matter
    int count_gt = 0, count_le = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) count_gt++;      // GT_EXPR in conditional
        if (a[i] <= b[i]) count_le++;     // LE_EXPR in conditional
    }
    printf("Count a[i] > b[i]: %d\n", count_gt);
    printf("Count a[i] <= b[i]: %d\n", count_le);
    
    return sum > 0 ? 0 : 1;  // Return value depends on comparisons
}
