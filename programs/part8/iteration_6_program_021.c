#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    int ALIGNED a[N];
    int ALIGNED b[N];
    int ALIGNED result_gt[N];
    int ALIGNED result_le[N];
    int ALIGNED result_lt[N];
    int ALIGNED result_ge[N];
    
    // Initialize arrays with distinct values
    for (int i = 0; i < N; i++) {
        a[i] = i * 3 + 1;      // 1, 4, 7, 10, ...
        b[i] = i * 2 + 2;      // 2, 4, 6, 8, ...
    }
    
    int checksum = 0;
    
    // Loop 1: GT_EXPR (>) - should trigger transformation for greater-than
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // GT_EXPR
    }
    
    // Loop 2: LE_EXPR (<=) - should trigger transformation for less-or-equal
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i]; // LE_EXPR
    }
    
    // Loop 3: LT_EXPR (<) - should trigger transformation for less-than
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];  // LT_EXPR
    }
    
    // Loop 4: GE_EXPR (>=) - should trigger transformation for greater-or-equal
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i]; // GE_EXPR
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        checksum += result_gt[i] + result_le[i] + result_lt[i] + result_ge[i];
    }
    
    // Print checksum to ensure all computations are used
    printf("Checksum: %d\n", checksum);
    
    // Additional test with mixed comparisons in same loop
    int mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Mix of comparison operators in same loop
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
    
    int mixed_checksum = 0;
    for (int i = 0; i < N; i++) {
        mixed_checksum += mixed_results[i];
    }
    printf("Mixed checksum: %d\n", mixed_checksum);
    
    return checksum > 0 ? 0 : 1;
}
