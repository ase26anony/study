#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main(void) {
    // Declare aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int result_gt[N];
    ALIGNED int result_ge[N];
    ALIGNED int result_lt[N];
    ALIGNED int result_le[N];
    
    // Initialize arrays with distinct values to avoid trivial comparisons
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Pattern: 0, 3, 6, 9, ...
        b[i] = (i * 2) % 100;      // Pattern: 0, 2, 4, 6, ...
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) - should trigger vectorizable_comparison for GT_EXPR
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // Store comparison result
        sum += result_gt[i];         // Use result to prevent elimination
    }
    
    // Loop 2: GE_EXPR (>=) - should trigger vectorizable_comparison for GE_EXPR
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i]; // Store comparison result
        sum += result_ge[i];         // Use result to prevent elimination
    }
    
    // Loop 3: LT_EXPR (<) - should trigger vectorizable_comparison for LT_EXPR
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];  // Store comparison result
        sum += result_lt[i];         // Use result to prevent elimination
    }
    
    // Loop 4: LE_EXPR (<=) - should trigger vectorizable_comparison for LE_EXPR
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i]; // Store comparison result
        sum += result_le[i];         // Use result to prevent elimination
    }
    
    // Additional test: Mixed comparisons in same loop
    // This might trigger different code paths
    ALIGNED int result_mixed[N];
    for (int i = 0; i < N; i++) {
        // Use both > and <= in conditional expressions
        if (a[i] > b[i]) {
            result_mixed[i] = 1;
        } else if (a[i] <= b[i]) {
            result_mixed[i] = 2;
        } else {
            result_mixed[i] = 0;
        }
        sum += result_mixed[i];
    }
    
    // Use the results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    int verify_gt = 0, verify_le = 0;
    for (int i = 0; i < 10; i++) {
        verify_gt += result_gt[i];
        verify_le += result_le[i];
    }
    printf("First 10 GT results sum: %d\n", verify_gt);
    printf("First 10 LE results sum: %d\n", verify_le);
    
    return sum > 0 ? 0 : 1;  // Return different values based on results
}
