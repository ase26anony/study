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
        a[i] = (i * 3) % 100;      // Varying values 0-99
        b[i] = (i * 2) % 100;      // Different pattern 0-99
    }
    
    int checksum = 0;
    
    // Loop 1: GT_EXPR (greater than) - should trigger uncovered path for GT_EXPR
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // GT_EXPR
    }
    
    // Loop 2: GE_EXPR (greater than or equal) - should trigger uncovered path for GE_EXPR
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i]; // GE_EXPR
    }
    
    // Loop 3: LT_EXPR (less than) - should trigger uncovered path for LT_EXPR
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];  // LT_EXPR
    }
    
    // Loop 4: LE_EXPR (less than or equal) - should trigger uncovered path for LE_EXPR
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i]; // LE_EXPR
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        checksum += result_gt[i] + result_ge[i] + result_lt[i] + result_le[i];
    }
    
    // Print checksum to ensure all comparisons are executed
    printf("Checksum: %d\n", checksum);
    
    // Additional test with mixed comparisons in same loop
    ALIGNED int mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Mix different comparison types
        if (a[i] > b[i]) {          // GT_EXPR
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {  // LE_EXPR
            mixed_results[i] = 2;
        } else {
            mixed_results[i] = 0;
        }
    }
    
    // Use mixed results
    int mixed_checksum = 0;
    for (int i = 0; i < N; i++) {
        mixed_checksum += mixed_results[i];
    }
    printf("Mixed checksum: %d\n", mixed_checksum);
    
    return (checksum > 0 && mixed_checksum > 0) ? 0 : 1;
}
