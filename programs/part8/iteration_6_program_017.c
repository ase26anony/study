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
    
    // Initialize with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>)
    // This should trigger the transformation for greater-than comparisons
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];
        sum += result_gt[i];  // Prevent dead code elimination
    }
    
    // Loop 2: GE_EXPR (>=)
    // This should trigger the transformation for greater-or-equal comparisons
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i];
        sum += result_ge[i];
    }
    
    // Loop 3: LT_EXPR (<)
    // This should trigger the transformation for less-than comparisons
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];
        sum += result_lt[i];
    }
    
    // Loop 4: LE_EXPR (<=)
    // This should trigger the transformation for less-or-equal comparisons
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i];
        sum += result_le[i];
    }
    
    // Use results to prevent optimization
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    int verify = 0;
    for (int i = 0; i < 10; i++) {
        verify += result_gt[i] + result_ge[i] + result_lt[i] + result_le[i];
    }
    printf("First 10 elements verification: %d\n", verify);
    
    return sum > 0 ? 0 : 1;
}
