#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    int ALIGNED a[N], b[N];
    int ALIGNED gt_results[N], ge_results[N];
    int ALIGNED lt_results[N], le_results[N];
    
    // Initialize arrays with distinct patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Values 0-99
        b[i] = (i * 7) % 100;      // Different pattern 0-99
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) and GE_EXPR (>=) comparisons
    for (int i = 0; i < N; i++) {
        // These should trigger GT_EXPR and GE_EXPR transformations
        gt_results[i] = a[i] > b[i];    // GT_EXPR
        ge_results[i] = a[i] >= b[i];   // GE_EXPR
    }
    
    // Loop 2: LT_EXPR (<) and LE_EXPR (<=) comparisons  
    for (int i = 0; i < N; i++) {
        // These should trigger LT_EXPR and LE_EXPR transformations
        lt_results[i] = a[i] < b[i];    // LT_EXPR
        le_results[i] = a[i] <= b[i];   // LE_EXPR
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        sum += gt_results[i] + ge_results[i] + lt_results[i] + le_results[i];
    }
    
    // Print checksum to ensure all comparisons are executed
    printf("Checksum: %d\n", sum);
    
    // Additional test with mixed comparisons in same loop
    int ALIGNED mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Mix different comparison types in conditional
        if (a[i] > b[i]) {          // GT_EXPR
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {  // LE_EXPR  
            mixed_results[i] = 2;
        } else {
            mixed_results[i] = 0;
        }
    }
    
    // Use mixed results
    int mixed_sum = 0;
    for (int i = 0; i < N; i++) {
        mixed_sum += mixed_results[i];
    }
    printf("Mixed checksum: %d\n", mixed_sum);
    
    return (sum > 2000 && mixed_sum > 1000) ? 0 : 1;
}
