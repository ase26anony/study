#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a[N], b[N];
    ALIGNED int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    ALIGNED int mixed_result[N];
    
    int sum = 0;
    
    // Initialize arrays with distinct values
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Varying pattern
        b[i] = (i * 2) % 100;      // Different pattern
    }
    
    // Loop 1: Greater-than comparisons (GT_EXPR)
    // This should trigger the GT_EXPR transformation path
    for (int i = 0; i < N; i++) {
        gt_result[i] = a[i] > b[i];  // GT_EXPR
    }
    
    // Loop 2: Greater-or-equal comparisons (GE_EXPR)
    // This should trigger the GE_EXPR transformation path
    for (int i = 0; i < N; i++) {
        ge_result[i] = a[i] >= b[i]; // GE_EXPR
    }
    
    // Loop 3: Less-than comparisons (LT_EXPR)
    // This should trigger the LT_EXPR transformation path
    for (int i = 0; i < N; i++) {
        lt_result[i] = a[i] < b[i];  // LT_EXPR
    }
    
    // Loop 4: Less-or-equal comparisons (LE_EXPR)
    // This should trigger the LE_EXPR transformation path
    for (int i = 0; i < N; i++) {
        le_result[i] = a[i] <= b[i]; // LE_EXPR
    }
    
    // Loop 5: Mixed comparisons in same loop
    // Tests multiple comparison types together
    for (int i = 0; i < N; i++) {
        int cmp1 = a[i] > b[i];    // GT_EXPR
        int cmp2 = a[i] <= b[i];   // LE_EXPR
        mixed_result[i] = cmp1 | cmp2;
    }
    
    // Loop 6: Comparisons with scalar threshold
    // Tests different operand patterns
    int threshold = 50;
    for (int i = 0; i < N; i++) {
        // Mix of array-array and array-scalar comparisons
        if (a[i] > b[i]) {         // GT_EXPR
            sum += 1;
        }
        if (a[i] <= threshold) {   // LE_EXPR with scalar
            sum += 2;
        }
    }
    
    // Loop 7: While loop with comparison
    // Different loop structure
    int j = 0;
    while (j < N) {
        if (a[j] < b[j]) {         // LT_EXPR
            sum += 3;
        }
        j++;
    }
    
    // Use results to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i] + mixed_result[i];
    }
    checksum += sum;
    
    printf("Checksum: %d\n", checksum);
    
    // Return value based on comparisons to ensure they're not optimized away
    return (checksum > 0) ? 0 : 1;
}
