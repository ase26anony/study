#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int result_gt[N];
    ALIGNED int result_ge[N];
    ALIGNED int result_lt[N];
    ALIGNED int result_le[N];
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Values 0-99
        b[i] = (i * 7) % 100;      // Different pattern
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) transformation
    // This should trigger the GT_EXPR -> BIT_AND_EXPR/BIT_NOT_EXPR path
    for (int i = 0; i < N; i++) {
        result_gt[i] = (a[i] > b[i]);  // GT_EXPR
    }
    
    // Loop 2: GE_EXPR (>=) transformation  
    // This should trigger the GE_EXPR -> BIT_AND_EXPR/BIT_IOR_EXPR path
    for (int i = 0; i < N; i++) {
        result_ge[i] = (a[i] >= b[i]); // GE_EXPR
    }
    
    // Loop 3: LT_EXPR (<) transformation
    // This should trigger the LT_EXPR -> BIT_AND_EXPR/BIT_NOT_EXPR path
    // with potential operand swapping
    for (int i = 0; i < N; i++) {
        result_lt[i] = (a[i] < b[i]);  // LT_EXPR
    }
    
    // Loop 4: LE_EXPR (<=) transformation
    // This should trigger the LE_EXPR -> BIT_AND_EXPR/BIT_IOR_EXPR path
    // with potential operand swapping
    for (int i = 0; i < N; i++) {
        result_le[i] = (a[i] <= b[i]); // LE_EXPR
    }
    
    // Use results to prevent dead code elimination
    // Compute checksums to ensure all comparisons are executed
    int checksum_gt = 0, checksum_ge = 0, checksum_lt = 0, checksum_le = 0;
    
    for (int i = 0; i < N; i++) {
        checksum_gt += result_gt[i];
        checksum_ge += result_ge[i];
        checksum_lt += result_lt[i];
        checksum_le += result_le[i];
    }
    
    // Print checksums (prevents optimization)
    printf("GT checksum: %d\n", checksum_gt);
    printf("GE checksum: %d\n", checksum_ge);
    printf("LT checksum: %d\n", checksum_lt);
    printf("LE checksum: %d\n", checksum_le);
    
    // Also test with scalar comparisons in conditional branches
    // This provides alternative patterns for the vectorizer
    int count_gt = 0, count_le = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {      // GT_EXPR in conditional
            count_gt++;
        }
        if (a[i] <= b[i]) {     // LE_EXPR in conditional
            count_le++;
        }
    }
    
    printf("Count GT: %d\n", count_gt);
    printf("Count LE: %d\n", count_le);
    
    // Return value based on comparisons to ensure they matter
    return (checksum_gt > 0 && checksum_lt > 0) ? 0 : 1;
}
