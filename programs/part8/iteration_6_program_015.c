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
        a[i] = (i * 3) % 100;      // Varying values 0-99
        b[i] = (i * 7) % 100;      // Different pattern 0-99
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) - should trigger GT_EXPR transformation
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // GT_EXPR
        sum += result_gt[i];         // Prevent elimination
    }
    
    // Loop 2: LE_EXPR (<=) - should trigger LE_EXPR transformation  
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i]; // LE_EXPR
        sum += result_le[i];         // Prevent elimination
    }
    
    // Loop 3: LT_EXPR (<) - should trigger LT_EXPR transformation
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];  // LT_EXPR
        sum += result_lt[i];         // Prevent elimination
    }
    
    // Loop 4: GE_EXPR (>=) - should trigger GE_EXPR transformation
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i]; // GE_EXPR
        sum += result_ge[i];         // Prevent elimination
    }
    
    // Additional test with mixed comparisons in same loop
    // This might trigger different transformation paths
    int mixed_results[N];
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {           // GT_EXPR
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {   // LE_EXPR  
            mixed_results[i] = 2;
        } else {
            mixed_results[i] = 0;
        }
        sum += mixed_results[i];
    }
    
    // Use results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    printf("Sample comparisons (a[i] > b[i]):\n");
    for (int i = 0; i < 10; i++) {
        printf("  a[%d]=%d, b[%d]=%d, gt=%d, le=%d, lt=%d, ge=%d\n",
               i, a[i], i, b[i], 
               result_gt[i], result_le[i], result_lt[i], result_ge[i]);
    }
    
    return sum > 0 ? 0 : 1;  // Return non-zero if all comparisons false
}
