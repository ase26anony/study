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
    
    // Declare destination arrays
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than comparisons (GT_EXPR)
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
    
    // Loop 2: Greater-than-or-equal comparisons (GE_EXPR)
    for (int i = 0; i < N; ++i) {
        // Different arrays to avoid dependencies
        if (b[i] >= c[i]) {
            ge_result[i] = b[i];
        } else if (b[i] >= 25) {  // Constant RHS
            ge_result[i] = 25;
        } else {
            ge_result[i] = 0;
        }
    }
    
    // Loop 3: Less-than comparisons (LT_EXPR)
    for (int i = 0; i < N; ++i) {
        // Use different operand order to test std::swap logic
        if (c[i] < a[i]) {
            lt_result[i] = c[i] * 2;
        } else if (c[i] < 75) {  // Constant RHS
            lt_result[i] = c[i];
        } else {
            lt_result[i] = -1;
        }
    }
    
    // Loop 4: Less-than-or-equal comparisons (LE_EXPR)
    for (int i = 0; i < N; ++i) {
        // Mix different integer types (implicit conversion)
        unsigned short us_val = (unsigned short)(i % 256);
        if (us_val <= (unsigned short)a[i]) {
            le_result[i] = us_val + a[i];
        } else if (us_val <= 128) {  // Constant RHS
            le_result[i] = us_val;
        } else {
            le_result[i] = 0;
        }
    }
    
    // Additional test with long integer type
    long long_result[N];
    for (int i = 0; i < N; ++i) {
        // Test with long type
        long val = (long)a[i] * 1000L;
        if (val <= 50000L) {
            long_result[i] = val;
        } else {
            long_result[i] = 50000L;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    unsigned long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i] + long_result[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    
    return 0;
}
