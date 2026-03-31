#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    int ALIGNED a[N];
    int ALIGNED b[N];
    int ALIGNED results_gt[N];
    int ALIGNED results_le[N];
    int ALIGNED results_lt[N];
    int ALIGNED results_ge[N];
    
    // Initialize arrays with distinct values
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Varying values 0-99
        b[i] = (i * 7) % 100;      // Different pattern 0-99
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (greater than) - should trigger uncovered GT_EXPR handling
    for (int i = 0; i < N; i++) {
        results_gt[i] = a[i] > b[i];  // GT_EXPR
    }
    
    // Loop 2: LE_EXPR (less than or equal) - should trigger uncovered LE_EXPR handling
    for (int i = 0; i < N; i++) {
        results_le[i] = a[i] <= b[i]; // LE_EXPR
    }
    
    // Loop 3: LT_EXPR (less than) - should trigger uncovered LT_EXPR handling
    for (int i = 0; i < N; i++) {
        results_lt[i] = a[i] < b[i];  // LT_EXPR
    }
    
    // Loop 4: GE_EXPR (greater than or equal) - should trigger uncovered GE_EXPR handling
    for (int i = 0; i < N; i++) {
        results_ge[i] = a[i] >= b[i]; // GE_EXPR
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        sum += results_gt[i] + results_le[i] + results_lt[i] + results_ge[i];
    }
    
    // Print checksum to ensure all comparisons are executed
    printf("Checksum: %d\n", sum);
    
    // Additional test with mixed comparisons in one loop
    int ALIGNED mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Mix GT and LE in conditional expressions
        if (a[i] > b[i]) {          // GT_EXPR
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {  // LE_EXPR
            mixed_results[i] = 2;
        }
        
        // Also include LT and GE in arithmetic context
        mixed_results[i] += (a[i] < b[i]) ? 10 : 0;   // LT_EXPR
        mixed_results[i] += (a[i] >= b[i]) ? 20 : 0;  // GE_EXPR
    }
    
    int mixed_sum = 0;
    for (int i = 0; i < N; i++) {
        mixed_sum += mixed_results[i];
    }
    printf("Mixed checksum: %d\n", mixed_sum);
    
    return (sum > 0 && mixed_sum > 0) ? 0 : 1;
}
