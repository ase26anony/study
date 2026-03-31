#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int result_gt[N];
    ALIGNED int result_ge[N];
    ALIGNED int result_lt[N];
    ALIGNED int result_le[N];
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = i * 2;
        b[i] = i * 3 - 100;  // Creates mix of true/false comparisons
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) - should trigger the transformation
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];
        sum += result_gt[i];  // Prevent dead code elimination
    }
    
    // Loop 2: GE_EXPR (>=) - should trigger the transformation
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i];
        sum += result_ge[i];
    }
    
    // Loop 3: LT_EXPR (<) - should trigger the transformation
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];
        sum += result_lt[i];
    }
    
    // Loop 4: LE_EXPR (<=) - should trigger the transformation
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i];
        sum += result_le[i];
    }
    
    // Additional test with mixed comparisons in same loop
    // This might trigger different optimization paths
    ALIGNED int mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Use different comparisons based on index
        if (i % 4 == 0) {
            mixed_results[i] = a[i] > b[i];      // GT_EXPR
        } else if (i % 4 == 1) {
            mixed_results[i] = a[i] >= b[i];     // GE_EXPR
        } else if (i % 4 == 2) {
            mixed_results[i] = a[i] < b[i];      // LT_EXPR
        } else {
            mixed_results[i] = a[i] <= b[i];     // LE_EXPR
        }
        sum += mixed_results[i];
    }
    
    // Use the results to prevent optimization
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    int verify_count = 0;
    for (int i = 0; i < 10; i++) {
        if (result_gt[i] != (a[i] > b[i])) verify_count++;
        if (result_ge[i] != (a[i] >= b[i])) verify_count++;
        if (result_lt[i] != (a[i] < b[i])) verify_count++;
        if (result_le[i] != (a[i] <= b[i])) verify_count++;
    }
    
    if (verify_count > 0) {
        printf("Verification failed on %d comparisons\n", verify_count);
        return 1;
    }
    
    return 0;
}
