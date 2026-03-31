#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    int ALIGNED a[N];
    int ALIGNED b[N];
    int ALIGNED result_gt[N];
    int ALIGNED result_ge[N];
    int ALIGNED result_lt[N];
    int ALIGNED result_le[N];
    
    // Initialize arrays with distinct patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Pattern: 0, 3, 6, 9, ...
        b[i] = (i * 2) % 100;      // Pattern: 0, 2, 4, 6, ...
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (greater than) transformation
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // Should trigger GT_EXPR vectorization
        sum += result_gt[i];
    }
    
    // Loop 2: GE_EXPR (greater than or equal) transformation  
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i]; // Should trigger GE_EXPR vectorization
        sum += result_ge[i];
    }
    
    // Loop 3: LT_EXPR (less than) transformation
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];  // Should trigger LT_EXPR vectorization
        sum += result_lt[i];
    }
    
    // Loop 4: LE_EXPR (less than or equal) transformation
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i]; // Should trigger LE_EXPR vectorization
        sum += result_le[i];
    }
    
    // Additional test with mixed comparisons in same loop
    int mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Mix of different comparison types
        if (a[i] > b[i]) {
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {
            mixed_results[i] = -1;
        } else {
            mixed_results[i] = 0;
        }
        sum += mixed_results[i];
    }
    
    // Use results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Verify a few values
    printf("Sample comparisons (a[i] vs b[i]):\n");
    for (int i = 0; i < 10; i++) {
        printf("a[%d]=%d, b[%d]=%d: gt=%d, ge=%d, lt=%d, le=%d\n",
               i, a[i], i, b[i], 
               result_gt[i], result_ge[i], result_lt[i], result_le[i]);
    }
    
    return sum != 0 ? 0 : 1;
}
