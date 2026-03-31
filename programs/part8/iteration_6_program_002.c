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
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Pattern: 0, 3, 6, 9, ...
        b[i] = (i * 2) % 100;      // Pattern: 0, 2, 4, 6, ...
    }
    
    int checksum = 0;
    
    // Loop 1: GT_EXPR (>) - should trigger transformation for greater-than
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // Scalar comparison to be vectorized
    }
    
    // Loop 2: LE_EXPR (<=) - should trigger transformation for less-or-equal
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i]; // Different comparison operator
    }
    
    // Loop 3: LT_EXPR (<) - should trigger transformation for less-than
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];
    }
    
    // Loop 4: GE_EXPR (>=) - should trigger transformation for greater-or-equal
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i];
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        checksum += result_gt[i] + result_le[i] + result_lt[i] + result_ge[i];
    }
    
    // Print checksum to ensure all computations are used
    printf("Checksum: %d\n", checksum);
    
    // Additional verification prints (optional)
    printf("Sample comparisons at i=0: a=%d, b=%d\n", a[0], b[0]);
    printf("GT: %d, LE: %d, LT: %d, GE: %d\n", 
           result_gt[0], result_le[0], result_lt[0], result_ge[0]);
    
    return checksum == 0 ? 0 : 1;
}
