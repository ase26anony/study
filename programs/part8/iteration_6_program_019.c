#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    int ALIGNED a[N], b[N];
    int ALIGNED gt_result[N], le_result[N];
    int ALIGNED lt_result[N], ge_result[N];
    
    // Initialize arrays with distinct values
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Varying values 0-99
        b[i] = (i * 7) % 100;      // Different pattern 0-99
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) transformation
    // This should trigger the GT_EXPR path in vectorizable_comparison
    for (int i = 0; i < N; i++) {
        gt_result[i] = a[i] > b[i];  // Scalar comparison to be vectorized
    }
    
    // Loop 2: LE_EXPR (<=) transformation  
    // This should trigger the LE_EXPR path with potential operand swapping
    for (int i = 0; i < N; i++) {
        le_result[i] = a[i] <= b[i];  // Different comparison type
    }
    
    // Loop 3: LT_EXPR (<) transformation
    // Additional loop to cover LT_EXPR
    for (int i = 0; i < N; i++) {
        lt_result[i] = a[i] < b[i];
    }
    
    // Loop 4: GE_EXPR (>=) transformation
    // Additional loop to cover GE_EXPR
    for (int i = 0; i < N; i++) {
        ge_result[i] = a[i] >= b[i];
    }
    
    // Use results to prevent dead code elimination
    // Compute checksums that depend on all comparisons
    int checksum_gt = 0, checksum_le = 0;
    int checksum_lt = 0, checksum_ge = 0;
    
    for (int i = 0; i < N; i++) {
        checksum_gt += gt_result[i] * i;
        checksum_le += le_result[i] * i;
        checksum_lt += lt_result[i] * i;
        checksum_ge += ge_result[i] * i;
    }
    
    // Final result depends on all comparisons
    int final_result = checksum_gt + checksum_le + checksum_lt + checksum_ge;
    
    // Print something to ensure side effects
    printf("Vectorization test result: %d\n", final_result % 1000);
    
    return final_result != 0 ? 0 : 1;
}
