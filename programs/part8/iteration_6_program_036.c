#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int result_gt[N];
    ALIGNED int result_le[N];
    ALIGNED int result_lt[N];
    ALIGNED int result_ge[N];
    
    // Initialize arrays with pattern to ensure non-trivial comparisons
    for (int i = 0; i < N; i++) {
        a[i] = i * 2;
        b[i] = i * 2 + (i % 3) - 1;  // Creates mix of true/false comparisons
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) comparison
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // Should trigger GT_EXPR path
        sum += result_gt[i];
    }
    
    // Loop 2: LE_EXPR (<=) comparison  
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i];  // Should trigger LE_EXPR path
        sum += result_le[i];
    }
    
    // Loop 3: LT_EXPR (<) comparison
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];  // Should trigger LT_EXPR path
        sum += result_lt[i];
    }
    
    // Loop 4: GE_EXPR (>=) comparison
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i];  // Should trigger GE_EXPR path
        sum += result_ge[i];
    }
    
    // Additional test with mixed comparisons in same loop
    ALIGNED int mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Mix of different comparison types
        if (a[i] > b[i]) {
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {
            mixed_results[i] = 2;
        } else if (a[i] < b[i]) {
            mixed_results[i] = 3;
        } else if (a[i] >= b[i]) {
            mixed_results[i] = 4;
        }
        sum += mixed_results[i];
    }
    
    // Use results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    int verify = 0;
    for (int i = 0; i < 10; i++) {
        verify += result_gt[i] + result_le[i] + result_lt[i] + result_ge[i];
    }
    printf("First 10 elements verification: %d\n", verify);
    
    return sum > 0 ? 0 : 1;
}
