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
        a[i] = i * 2;
        b[i] = i * 2 + 1;  // Ensures a[i] < b[i] for all i
    }
    
    // Make some elements equal to test equality cases
    for (int i = 0; i < N; i += 8) {
        a[i] = b[i];
    }
    
    int checksum = 0;
    
    // Loop 1: GT_EXPR (greater than) - should use BIT_AND_EXPR with NOT
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // GT_EXPR
    }
    
    // Loop 2: LE_EXPR (less than or equal) - should use BIT_IOR_EXPR with NOT
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Loop 3: LT_EXPR (less than) - should use BIT_AND_EXPR
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];  // LT_EXPR
    }
    
    // Loop 4: GE_EXPR (greater than or equal) - should use BIT_IOR_EXPR
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i];  // GE_EXPR
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        checksum += result_gt[i] + result_le[i] + result_lt[i] + result_ge[i];
    }
    
    // Print checksum to ensure code isn't optimized away
    printf("Checksum: %d\n", checksum);
    
    // Additional test with scalar comparison to cover different patterns
    int threshold = N/2;
    int count_gt = 0, count_le = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] > threshold) count_gt++;   // GT_EXPR with scalar
        if (a[i] <= threshold) count_le++;  // LE_EXPR with scalar
    }
    
    printf("Count > threshold: %d\n", count_gt);
    printf("Count <= threshold: %d\n", count_le);
    
    return checksum > 0 ? 0 : 1;
}
