#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    int ALIGNED a[N], b[N];
    int ALIGNED gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize arrays with distinct values
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Values 0-99
        b[i] = (i * 7) % 100;      // Different pattern
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) and GE_EXPR (>=) comparisons
    for (int i = 0; i < N; i++) {
        // These should trigger GT_EXPR and GE_EXPR transformations
        gt_result[i] = a[i] > b[i];    // Greater than
        ge_result[i] = a[i] >= b[i];   // Greater than or equal
    }
    
    // Loop 2: LT_EXPR (<) and LE_EXPR (<=) comparisons  
    for (int i = 0; i < N; i++) {
        // These should trigger LT_EXPR and LE_EXPR transformations
        lt_result[i] = a[i] < b[i];    // Less than
        le_result[i] = a[i] <= b[i];   // Less than or equal
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        sum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    
    // Print checksum to ensure all comparisons are executed
    printf("Checksum: %d\n", sum);
    
    // Additional test with mixed comparisons in same loop
    int ALIGNED mixed_result[N];
    for (int i = 0; i < N; i++) {
        // Mix different comparison types in same loop
        if (a[i] > b[i]) {
            mixed_result[i] = 1;
        } else if (a[i] <= b[i]) {
            mixed_result[i] = 2;
        } else {
            mixed_result[i] = 0;
        }
    }
    
    // Use mixed results
    int mixed_sum = 0;
    for (int i = 0; i < N; i++) {
        mixed_sum += mixed_result[i];
    }
    printf("Mixed checksum: %d\n", mixed_sum);
    
    return (sum > 0 && mixed_sum > 0) ? 0 : 1;
}
