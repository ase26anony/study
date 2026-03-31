#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int result_gt[N];
    ALIGNED int result_ge[N];
    ALIGNED int result_lt[N];
    ALIGNED int result_le[N];
    
    // Initialize with distinct values to avoid trivial comparisons
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>)
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];
        sum += result_gt[i];  // Prevent dead code elimination
    }
    
    // Loop 2: GE_EXPR (>=)
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i];
        sum += result_ge[i];
    }
    
    // Loop 3: LT_EXPR (<)
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];
        sum += result_lt[i];
    }
    
    // Loop 4: LE_EXPR (<=)
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i];
        sum += result_le[i];
    }
    
    // Mixed comparisons in same loop to test different patterns
    ALIGNED int mixed_results[N];
    for (int i = 0; i < N; i++) {
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
    
    // Use results to prevent optimization
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    int verify_sum = 0;
    for (int i = 0; i < 10; i++) {
        verify_sum += result_gt[i] + result_ge[i] + result_lt[i] + result_le[i];
    }
    printf("First 10 elements verification sum: %d\n", verify_sum);
    
    return sum > 0 ? 0 : 1;
}
