#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int result_gt[N];
    ALIGNED int result_le[N];
    ALIGNED int result_lt[N];
    ALIGNED int result_ge[N];
    
    // Initialize with distinct values to avoid trivial comparisons
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) - should trigger transformation for greater-than
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];
        sum += result_gt[i];  // Use result to prevent elimination
    }
    
    // Loop 2: LE_EXPR (<=) - should trigger transformation for less-or-equal
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i];
        sum += result_le[i];
    }
    
    // Loop 3: LT_EXPR (<) - should trigger transformation for less-than
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];
        sum += result_lt[i];
    }
    
    // Loop 4: GE_EXPR (>=) - should trigger transformation for greater-or-equal
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i];
        sum += result_ge[i];
    }
    
    // Additional test with mixed comparisons in same loop
    // This might trigger different transformation paths
    ALIGNED int result_mixed[N];
    for (int i = 0; i < N; i++) {
        // Mix of different comparison types
        if (i % 4 == 0) {
            result_mixed[i] = a[i] > b[i];    // GT_EXPR
        } else if (i % 4 == 1) {
            result_mixed[i] = a[i] <= b[i];   // LE_EXPR
        } else if (i % 4 == 2) {
            result_mixed[i] = a[i] < b[i];    // LT_EXPR
        } else {
            result_mixed[i] = a[i] >= b[i];   // GE_EXPR
        }
        sum += result_mixed[i];
    }
    
    // Use the results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    int verify = 0;
    for (int i = 0; i < 10; i++) {
        verify += result_gt[i] + result_le[i] + result_lt[i] + result_ge[i];
    }
    printf("Verification sum (first 10 elements): %d\n", verify);
    
    return sum > 0 ? 0 : 1;
}
