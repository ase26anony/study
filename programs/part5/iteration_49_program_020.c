#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays with distinct data
    int a[N], b[N], c[N];
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        c[i] = (i * 11) % 100;
    }
    
    // Declare destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Mix array-to-array and array-to-constant comparisons
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else if (a[i] > 50) {  // Constant RHS
            gt_result[i] = 2;
        } else {
            gt_result[i] = 0;
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Different arrays to avoid dependencies
        if (b[i] >= c[i]) {
            ge_result[i] = b[i];
        } else if (b[i] >= 25) {  // Constant RHS
            ge_result[i] = c[i];
        } else {
            ge_result[i] = 0;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Using different operand order than previous loops
        if (c[i] < a[i]) {
            lt_result[i] = c[i] * 2;
        } else if (c[i] < 75) {  // Constant RHS
            lt_result[i] = a[i] * 2;
        } else {
            lt_result[i] = 0;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Mix of comparisons with different RHS types
        if (a[i] <= b[i]) {
            le_result[i] = a[i] + b[i];
        } else if (a[i] <= 60) {  // Constant RHS
            le_result[i] = a[i] - b[i];
        } else {
            le_result[i] = 0;
        }
    }
    
    // Additional loops with unsigned types to test different integer widths
    unsigned short us_a[N], us_b[N];
    unsigned short us_result[N];
    
    for (int i = 0; i < N; ++i) {
        us_a[i] = (i * 5) % 256;
        us_b[i] = (i * 13) % 256;
    }
    
    // Loop 5: Greater-than with unsigned short
    for (int i = 0; i < N; ++i) {
        us_result[i] = (us_a[i] > us_b[i]) ? us_a[i] : us_b[i];
    }
    
    // Loop 6: Less-than-or-equal with unsigned short
    for (int i = 0; i < N; ++i) {
        if (us_a[i] <= 128) {  // Constant RHS
            us_result[i] += 1;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i] + us_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Verify with a simple test
    int verify_sum = 0;
    for (int i = 0; i < 10; ++i) {
        verify_sum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    printf("First 10 elements sum: %d\n", verify_sum);
    
    return 0;
}
