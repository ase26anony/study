#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays with distinct patterns
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Initialize with reproducible patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        c[i] = (i * 11) % 100;
        d[i] = (i * 5) % 256;
        e[i] = (i * 13) % 1000;
    }
    
    // Destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            gt_result[i] = 0;
        }
        
        // Additional comparison with constant to test different RHS
        if (d[i] > 128) {
            gt_result[i] += 2;  // Add to existing value
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (a[i] >= c[i]) {
            ge_result[i] = 1;
        } else {
            ge_result[i] = 0;
        }
        
        // Comparison with constant
        if (e[i] >= 500) {
            ge_result[i] += 10;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (b[i] < a[i]) {
            lt_result[i] = 1;
        } else {
            lt_result[i] = 0;
        }
        
        // Comparison with constant
        if (c[i] < 50) {
            lt_result[i] += 3;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (c[i] <= b[i]) {
            le_result[i] = 1;
        } else {
            le_result[i] = 0;
        }
        
        // Comparison with constant
        if (d[i] <= 100) {
            le_result[i] += 5;
        }
    }
    
    // Additional loops with different integer types to ensure coverage
    // Using different data types to test various vectorization paths
    
    // Loop 5: GT_EXPR with unsigned short
    unsigned short us_gt_result[N];
    for (int i = 0; i < N; i++) {
        if (d[i] > (unsigned short)(i % 128)) {
            us_gt_result[i] = 255;
        } else {
            us_gt_result[i] = 0;
        }
    }
    
    // Loop 6: LE_EXPR with long
    long long_le_result[N];
    for (int i = 0; i < N; i++) {
        if (e[i] <= (long)(i * 2)) {
            long_le_result[i] = 999;
        } else {
            long_le_result[i] = 0;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
        checksum += us_gt_result[i] + long_le_result[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    
    // Print a few values to verify execution
    printf("Sample values (first 5):\n");
    for (int i = 0; i < 5; i++) {
        printf("  [%d]: gt=%d, ge=%d, lt=%d, le=%d\n", 
               i, gt_result[i], ge_result[i], lt_result[i], le_result[i]);
    }
    
    return 0;
}
