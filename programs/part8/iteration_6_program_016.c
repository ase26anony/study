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
        b[i] = (i * 5) % 100;
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) - should trigger vectorization
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // GT_EXPR
    }
    
    // Loop 2: GE_EXPR (>=) - should trigger vectorization
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i]; // GE_EXPR
    }
    
    // Loop 3: LT_EXPR (<) - should trigger vectorization
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];  // LT_EXPR
    }
    
    // Loop 4: LE_EXPR (<=) - should trigger vectorization
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i]; // LE_EXPR
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        sum += result_gt[i] + result_ge[i] + result_lt[i] + result_le[i];
    }
    
    // Print checksum to ensure all computations are used
    printf("Checksum: %d\n", sum);
    
    return sum == 0 ? 1 : 0;
}
