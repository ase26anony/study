#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED char result_gt[N];
    ALIGNED char result_le[N];
    ALIGNED char result_lt[N];
    ALIGNED char result_ge[N];
    
    // Initialize arrays with varying patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    int sum = 0;
    
    // Loop 1: Use GT_EXPR (>) comparison
    for (int i = 0; i < N; i++) {
        result_gt[i] = (a[i] > b[i]);  // GT_EXPR
    }
    
    // Loop 2: Use LE_EXPR (<=) comparison  
    for (int i = 0; i < N; i++) {
        result_le[i] = (a[i] <= b[i]);  // LE_EXPR
    }
    
    // Loop 3: Use LT_EXPR (<) comparison
    for (int i = 0; i < N; i++) {
        result_lt[i] = (a[i] < b[i]);  // LT_EXPR
    }
    
    // Loop 4: Use GE_EXPR (>=) comparison
    for (int i = 0; i < N; i++) {
        result_ge[i] = (a[i] >= b[i]);  // GE_EXPR
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        sum += result_gt[i] + result_le[i] + result_lt[i] + result_ge[i];
    }
    
    // Print checksum to ensure computations aren't optimized away
    printf("Checksum: %d\n", sum);
    
    return sum == 0 ? 1 : 0;
}
