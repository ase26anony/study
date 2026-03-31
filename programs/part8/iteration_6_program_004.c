#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Aligned arrays to help vectorization
    int ALIGNED a[N];
    int ALIGNED b[N];
    int ALIGNED results_gt[N];
    int ALIGNED results_le[N];
    int ALIGNED results_lt[N];
    int ALIGNED results_ge[N];
    
    // Initialize with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) transformation
    for (int i = 0; i < N; i++) {
        results_gt[i] = a[i] > b[i];  // Should trigger GT_EXPR handling
        sum += results_gt[i];
    }
    
    // Loop 2: LE_EXPR (<=) transformation  
    for (int i = 0; i < N; i++) {
        results_le[i] = a[i] <= b[i];  // Should trigger LE_EXPR handling
        sum += results_le[i];
    }
    
    // Loop 3: LT_EXPR (<) transformation
    for (int i = 0; i < N; i++) {
        results_lt[i] = a[i] < b[i];  // Should trigger LT_EXPR handling
        sum += results_lt[i];
    }
    
    // Loop 4: GE_EXPR (>=) transformation
    for (int i = 0; i < N; i++) {
        results_ge[i] = a[i] >= b[i];  // Should trigger GE_EXPR handling
        sum += results_ge[i];
    }
    
    // Additional test with mixed comparisons in same loop
    int mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Mix GT and LE in conditional
        if (a[i] > b[i]) {
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {
            mixed_results[i] = 2;
        } else {
            mixed_results[i] = 0;
        }
        sum += mixed_results[i];
    }
    
    // Use results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    int verify_sum = 0;
    for (int i = 0; i < 10; i++) {
        verify_sum += results_gt[i] + results_le[i] + results_lt[i] + results_ge[i];
    }
    printf("First 10 elements verification: %d\n", verify_sum);
    
    return sum > 0 ? 0 : 1;
}
