#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int result_gt[N];
    ALIGNED int result_ge[N];
    ALIGNED int result_lt[N];
    ALIGNED int result_le[N];
    
    // Initialize arrays with distinct values
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Varying pattern
        b[i] = (i * 2) % 100;      // Different pattern
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (greater than) transformation
    // This should trigger the GT_EXPR -> vectorized comparison path
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // GT_EXPR
    }
    
    // Loop 2: GE_EXPR (greater than or equal) transformation  
    // This should trigger the GE_EXPR -> vectorized comparison path
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i]; // GE_EXPR
    }
    
    // Loop 3: LT_EXPR (less than) transformation
    // This should trigger the LT_EXPR -> vectorized comparison path
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];  // LT_EXPR
    }
    
    // Loop 4: LE_EXPR (less than or equal) transformation
    // This should trigger the LE_EXPR -> vectorized comparison path
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i]; // LE_EXPR
    }
    
    // Use results to prevent dead code elimination
    // Compute checksum to ensure all comparisons are executed
    for (int i = 0; i < N; i++) {
        sum += result_gt[i] + result_ge[i] + result_lt[i] + result_le[i];
    }
    
    // Print checksum to prevent optimization
    printf("Checksum: %d\n", sum);
    
    // Additional test with mixed comparisons in same loop
    // This tests the vectorizer's ability to handle multiple comparison types
    ALIGNED int mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Mix of different comparison operators
        if (a[i] > b[i]) {          // GT_EXPR
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {   // LE_EXPR
            mixed_results[i] = 2;
        } else {
            mixed_results[i] = 0;
        }
    }
    
    // Use mixed results
    int mixed_sum = 0;
    for (int i = 0; i < N; i++) {
        mixed_sum += mixed_results[i];
    }
    printf("Mixed checksum: %d\n", mixed_sum);
    
    return (sum > 0 && mixed_sum > 0) ? 0 : 1;
}
