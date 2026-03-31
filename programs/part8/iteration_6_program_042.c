#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED char gt_result[N];  // Results for greater-than comparisons
    ALIGNED char le_result[N];  // Results for less-or-equal comparisons
    ALIGNED char lt_result[N];  // Results for less-than comparisons
    ALIGNED char ge_result[N];  // Results for greater-or-equal comparisons
    
    // Initialize arrays with varying patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Values 0-99
        b[i] = (i * 7) % 100;      // Different pattern 0-99
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (greater than) and GE_EXPR (greater or equal)
    // This should trigger the transformation for > and >= comparisons
    for (int i = 0; i < N; i++) {
        // GT_EXPR: a[i] > b[i]
        gt_result[i] = (a[i] > b[i]);
        
        // GE_EXPR: a[i] >= b[i]
        ge_result[i] = (a[i] >= b[i]);
        
        // Use results to prevent elimination
        sum += gt_result[i] + ge_result[i];
    }
    
    // Loop 2: LT_EXPR (less than) and LE_EXPR (less or equal)
    // This should trigger the transformation for < and <= comparisons
    for (int i = 0; i < N; i++) {
        // LT_EXPR: a[i] < b[i]
        lt_result[i] = (a[i] < b[i]);
        
        // LE_EXPR: a[i] <= b[i]
        le_result[i] = (a[i] <= b[i]);
        
        // Use results to prevent elimination
        sum += lt_result[i] + le_result[i];
    }
    
    // Additional loop with mixed comparisons to ensure all paths are exercised
    // This uses different data types to potentially trigger different vectorization patterns
    ALIGNED short c[N];
    ALIGNED short d[N];
    ALIGNED char mixed_result[N];
    
    for (int i = 0; i < N; i++) {
        c[i] = (i * 5) % 256;
        d[i] = (i * 11) % 256;
    }
    
    // Mix of all four comparison types in one loop
    for (int i = 0; i < N; i++) {
        char temp = 0;
        if (c[i] > d[i])   temp |= 0x1;   // GT_EXPR
        if (c[i] >= d[i])  temp |= 0x2;   // GE_EXPR  
        if (c[i] < d[i])   temp |= 0x4;   // LT_EXPR
        if (c[i] <= d[i])  temp |= 0x8;   // LE_EXPR
        mixed_result[i] = temp;
        sum += temp;
    }
    
    // Final computation to use all results and prevent dead code elimination
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += gt_result[i] + le_result[i] + lt_result[i] + ge_result[i] + mixed_result[i];
    }
    
    // Print result to ensure code isn't optimized away
    printf("Result checksum: %d\n", final_sum);
    
    return (final_sum > 0) ? 0 : 1;
}
