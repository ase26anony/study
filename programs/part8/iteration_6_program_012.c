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
    
    int sum = 0;
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Varying values 0-99
        b[i] = (i * 7) % 100;      // Different pattern 0-99
    }
    
    // Loop 1: GT_EXPR (>) - should trigger GT_EXPR transformation
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // GT_EXPR
    }
    
    // Loop 2: LE_EXPR (<=) - should trigger LE_EXPR transformation  
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i]; // LE_EXPR
    }
    
    // Loop 3: LT_EXPR (<) - should trigger LT_EXPR transformation
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];  // LT_EXPR
    }
    
    // Loop 4: GE_EXPR (>=) - should trigger GE_EXPR transformation
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i]; // GE_EXPR
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        sum += result_gt[i] + result_le[i] + result_lt[i] + result_ge[i];
    }
    
    // Print checksum to ensure all computations are used
    printf("Checksum: %d\n", sum);
    
    // Additional test with mixed comparisons in same loop
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
