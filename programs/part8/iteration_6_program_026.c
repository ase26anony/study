#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGN __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    int ALIGN a[N];
    int ALIGN b[N];
    int ALIGN gt_result[N];
    int ALIGN le_result[N];
    int ALIGN lt_result[N];
    int ALIGN ge_result[N];
    
    // Initialize arrays with distinct values
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Pattern: 0, 3, 6, 9, ...
        b[i] = (i * 2) % 100;      // Pattern: 0, 2, 4, 6, ...
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) - should trigger transformation for greater-than
    for (int i = 0; i < N; i++) {
        gt_result[i] = a[i] > b[i];  // Store comparison result
    }
    
    // Loop 2: LE_EXPR (<=) - should trigger transformation for less-or-equal
    for (int i = 0; i < N; i++) {
        le_result[i] = a[i] <= b[i]; // Store comparison result
    }
    
    // Loop 3: LT_EXPR (<) - should trigger transformation for less-than
    for (int i = 0; i < N; i++) {
        lt_result[i] = a[i] < b[i];  // Store comparison result
    }
    
    // Loop 4: GE_EXPR (>=) - should trigger transformation for greater-or-equal
    for (int i = 0; i < N; i++) {
        ge_result[i] = a[i] >= b[i]; // Store comparison result
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        sum += gt_result[i] + le_result[i] + lt_result[i] + ge_result[i];
    }
    
    // Print checksum to ensure all comparisons are computed
    printf("Checksum: %d\n", sum);
    
    // Additional test with mixed comparisons in same loop
    int ALIGN mixed_result[N];
    for (int i = 0; i < N; i++) {
        // Mix two different comparison types in same expression
        mixed_result[i] = (a[i] > b[i]) && (a[i] <= 50);
    }
    
    int mixed_sum = 0;
    for (int i = 0; i < N; i++) {
        mixed_sum += mixed_result[i];
    }
    printf("Mixed checksum: %d\n", mixed_sum);
    
    return (sum > 0) ? 0 : 1;
}
