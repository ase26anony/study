#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED char result_gt[N];  // For > comparisons
    ALIGNED char result_le[N];  // For <= comparisons
    ALIGNED char result_lt[N];  // For < comparisons  
    ALIGNED char result_ge[N];  // For >= comparisons
    
    // Initialize arrays with varying patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Values 0-99
        b[i] = (i * 7) % 100;      // Different pattern 0-99
    }
    
    int checksum = 0;
    
    // Loop 1: GT_EXPR (>) comparisons
    // This should trigger the GT_EXPR path in vectorizable_comparison
    for (int i = 0; i < N; i++) {
        result_gt[i] = (a[i] > b[i]);  // GT_EXPR
    }
    
    // Loop 2: LE_EXPR (<=) comparisons  
    // This should trigger the LE_EXPR path in vectorizable_comparison
    for (int i = 0; i < N; i++) {
        result_le[i] = (a[i] <= b[i]);  // LE_EXPR
    }
    
    // Loop 3: LT_EXPR (<) comparisons
    // This should trigger the LT_EXPR path in vectorizable_comparison
    for (int i = 0; i < N; i++) {
        result_lt[i] = (a[i] < b[i]);  // LT_EXPR
    }
    
    // Loop 4: GE_EXPR (>=) comparisons
    // This should trigger the GE_EXPR path in vectorizable_comparison
    for (int i = 0; i < N; i++) {
        result_ge[i] = (a[i] >= b[i]);  // GE_EXPR
    }
    
    // Use results to prevent dead code elimination
    // Compute checksum from all comparison results
    for (int i = 0; i < N; i++) {
        checksum += result_gt[i] + result_le[i] + result_lt[i] + result_ge[i];
    }
    
    // Additional conditional with side effects
    int count_gt = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  // Another GT_EXPR in conditional context
            count_gt++;
        }
    }
    
    checksum += count_gt;
    
    // Print checksum to prevent optimization
    printf("Checksum: %d\n", checksum);
    
    // Verify all comparisons are consistent
    for (int i = 0; i < N; i++) {
        if ((result_gt[i] && result_le[i]) || (!result_gt[i] && !result_le[i])) {
            // Should be complementary
            printf("Inconsistent results at index %d\n", i);
            return 1;
        }
    }
    
    return 0;
}
