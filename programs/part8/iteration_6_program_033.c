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
    
    int sum = 0;
    
    // Initialize arrays with varying patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 2) % 100;
    }
    
    // Loop 1: GT_EXPR (>) - should trigger transformation for greater-than
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // GT_EXPR
    }
    
    // Loop 2: LE_EXPR (<=) - should trigger transformation for less-or-equal
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Loop 3: LT_EXPR (<) - should trigger transformation for less-than
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];  // LT_EXPR
    }
    
    // Loop 4: GE_EXPR (>=) - should trigger transformation for greater-or-equal
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i];  // GE_EXPR
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        sum += result_gt[i] + result_le[i] + result_lt[i] + result_ge[i];
    }
    
    printf("Checksum: %d\n", sum);
    
    // Additional test with mixed comparisons in one loop
    ALIGNED int mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Mix of different comparison types
        if (a[i] > b[i]) {          // GT_EXPR
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {  // LE_EXPR  
            mixed_results[i] = 2;
        } else if (a[i] < b[i]) {   // LT_EXPR
            mixed_results[i] = 3;
        } else if (a[i] >= b[i]) {  // GE_EXPR
            mixed_results[i] = 4;
        }
    }
    
    int mixed_sum = 0;
    for (int i = 0; i < N; i++) {
        mixed_sum += mixed_results[i];
    }
    printf("Mixed checksum: %d\n", mixed_sum);
    
    return (sum + mixed_sum) > 0 ? 0 : 1;
}
