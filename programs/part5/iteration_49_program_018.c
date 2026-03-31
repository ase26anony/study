#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define CONST_LIMIT 500

int main() {
    // Declare source arrays with different data patterns
    int a[N], b[N], c[N];
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize source arrays with reproducible pseudo-random data
    srand(42);
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = i;  // Simple linear pattern for constant comparison
    }
    
    // Loop 1: Greater-than (GT_EXPR) comparison
    // Comparison between two array elements
    for (int i = 0; i < N; i++) {
        // Conditional assignment based on a[i] > b[i]
        // This should trigger GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR transformation
        gt_result[i] = (a[i] > b[i]) ? 1 : 0;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparison  
    // Comparison between array element and constant
    for (int i = 0; i < N; i++) {
        // Conditional assignment based on a[i] >= CONST_LIMIT
        // This should trigger GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR transformation
        ge_result[i] = (a[i] >= CONST_LIMIT) ? 1 : 0;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparison
    // Comparison between two array elements with swapped operands pattern
    for (int i = 0; i < N; i++) {
        // Conditional assignment based on b[i] < a[i]
        // This should trigger LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR + swap transformation
        lt_result[i] = (b[i] < a[i]) ? 1 : 0;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparison
    // Mixed comparison: array vs array and array vs constant
    for (int i = 0; i < N; i++) {
        // Two comparisons combined with OR
        // First part: c[i] <= b[i] should trigger LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR + swap
        // Second part: a[i] <= CONST_LIMIT should also trigger the same transformation
        le_result[i] = (c[i] <= b[i] || a[i] <= CONST_LIMIT) ? 1 : 0;
    }
    
    // Compute checksum to ensure all loops have observable effects
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    // Additional verification: print first few results
    printf("First 10 results:\n");
    for (int i = 0; i < 10 && i < N; i++) {
        printf("i=%d: a=%d, b=%d, c=%d | gt=%d, ge=%d, lt=%d, le=%d\n",
               i, a[i], b[i], c[i], gt_result[i], ge_result[i], lt_result[i], le_result[i]);
    }
    
    return 0;
}
