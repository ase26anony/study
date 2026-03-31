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
        b[i] = (i * 7) % 100;      // Different pattern 0-99
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) - should trigger vectorization
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // Greater than comparison
        sum += result_gt[i];         // Prevent elimination
    }
    
    // Loop 2: GE_EXPR (>=) - should trigger vectorization
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i]; // Greater than or equal comparison
        sum += result_ge[i];         // Prevent elimination
    }
    
    // Loop 3: LT_EXPR (<) - should trigger vectorization
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];  // Less than comparison
        sum += result_lt[i];         // Prevent elimination
    }
    
    // Loop 4: LE_EXPR (<=) - should trigger vectorization
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i]; // Less than or equal comparison
        sum += result_le[i];         // Prevent elimination
    }
    
    // Mixed comparisons in a single loop to test different paths
    ALIGNED int mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Use different comparisons based on index
        if (i % 4 == 0) {
            mixed_results[i] = a[i] > b[i];    // GT_EXPR
        } else if (i % 4 == 1) {
            mixed_results[i] = a[i] >= b[i];   // GE_EXPR
        } else if (i % 4 == 2) {
            mixed_results[i] = a[i] < b[i];    // LT_EXPR
        } else {
            mixed_results[i] = a[i] <= b[i];   // LE_EXPR
        }
        sum += mixed_results[i];
    }
    
    // Use results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    int verify_sum = 0;
    for (int i = 0; i < 10; i++) {
        verify_sum += result_gt[i] + result_ge[i] + result_lt[i] + result_le[i];
    }
    printf("Verification sum (first 10 elements): %d\n", verify_sum);
    
    return sum > 0 ? 0 : 1;  // Return non-zero if all comparisons were false
}
