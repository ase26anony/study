#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED char gt_result[N];  // Store > comparisons
    ALIGNED char le_result[N];  // Store <= comparisons
    ALIGNED char lt_result[N];  // Store < comparisons  
    ALIGNED char ge_result[N];  // Store >= comparisons
    
    // Initialize with pattern that creates mixed comparison results
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = (i % 3) * 100 + (i % 5);
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) - should trigger transformation for greater-than
    for (int i = 0; i < N; i++) {
        gt_result[i] = a[i] > b[i];  // Vectorizable > comparison
        sum += gt_result[i];  // Use result to prevent elimination
    }
    
    // Loop 2: LE_EXPR (<=) - should trigger transformation for less-or-equal
    for (int i = 0; i < N; i++) {
        le_result[i] = a[i] <= b[i];  // Vectorizable <= comparison
        sum += le_result[i];  // Use result
    }
    
    // Loop 3: LT_EXPR (<) - should trigger transformation for less-than
    for (int i = 0; i < N; i++) {
        lt_result[i] = a[i] < b[i];  // Vectorizable < comparison
        sum += lt_result[i];  // Use result
    }
    
    // Loop 4: GE_EXPR (>=) - should trigger transformation for greater-or-equal
    for (int i = 0; i < N; i++) {
        ge_result[i] = a[i] >= b[i];  // Vectorizable >= comparison
        sum += ge_result[i];  // Use result
    }
    
    // Also test with swapped operands to cover operand swapping logic
    ALIGNED char swapped_result[N];
    for (int i = 0; i < N; i++) {
        // This might trigger operand swapping in the transformation
        swapped_result[i] = b[i] < a[i];  // Equivalent to a[i] > b[i]
        sum += swapped_result[i];
    }
    
    // Use results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    int verify = 0;
    for (int i = 0; i < 10; i++) {
        verify += gt_result[i] + le_result[i] + lt_result[i] + ge_result[i];
    }
    printf("Sample verification: %d\n", verify);
    
    return sum > 0 ? 0 : 1;
}
