#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    ALIGNED int a[N], b[N];
    ALIGNED int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Varying values 0-99
        b[i] = (i * 7) % 100;      // Different pattern 0-99
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) transformation
    for (int i = 0; i < N; i++) {
        gt_result[i] = a[i] > b[i];  // Should trigger GT_EXPR path
        sum += gt_result[i];
    }
    
    // Loop 2: GE_EXPR (>=) transformation  
    for (int i = 0; i < N; i++) {
        ge_result[i] = a[i] >= b[i]; // Should trigger GE_EXPR path
        sum += ge_result[i];
    }
    
    // Loop 3: LT_EXPR (<) transformation
    for (int i = 0; i < N; i++) {
        lt_result[i] = a[i] < b[i];  // Should trigger LT_EXPR path
        sum += lt_result[i];
    }
    
    // Loop 4: LE_EXPR (<=) transformation
    for (int i = 0; i < N; i++) {
        le_result[i] = a[i] <= b[i]; // Should trigger LE_EXPR path
        sum += le_result[i];
    }
    
    // Additional test with mixed comparisons in one loop
    // This might trigger different transformation patterns
    ALIGNED int mixed_result[N];
    for (int i = 0; i < N; i++) {
        // Use different comparisons based on index
        if (i % 4 == 0) {
            mixed_result[i] = a[i] > b[i];    // GT_EXPR
        } else if (i % 4 == 1) {
            mixed_result[i] = a[i] >= b[i];   // GE_EXPR
        } else if (i % 4 == 2) {
            mixed_result[i] = a[i] < b[i];    // LT_EXPR
        } else {
            mixed_result[i] = a[i] <= b[i];   // LE_EXPR
        }
        sum += mixed_result[i];
    }
    
    // Use results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    int verify_sum = 0;
    for (int i = 0; i < 10; i++) {
        verify_sum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    printf("First 10 elements verification sum: %d\n", verify_sum);
    
    return sum > 0 ? 0 : 1;
}
