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
    
    // Destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Mix array-to-array and array-to-constant comparisons
        if (a[i] > b[i]) {
            gt_result[i] = 1;  // True case
        } else if (a[i] > 50) {  // Comparison with loop-invariant constant
            gt_result[i] = 2;  // Different value for constant comparison
        } else {
            gt_result[i] = 0;  // False case
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Use different arrays to avoid dependencies
        if (b[i] >= c[i]) {
            ge_result[i] = b[i];  // Store actual value when true
        } else if (b[i] >= 25) {  // Comparison with constant
            ge_result[i] = -1;    // Special marker for constant comparison true
        } else {
            ge_result[i] = 0;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Use different operand order
        if (c[i] < a[i]) {
            lt_result[i] = c[i] * 2;  // Computation on true branch
        } else if (c[i] < 75) {       // Comparison with constant
            lt_result[i] = 1;
        } else {
            lt_result[i] = 0;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Mix different data types (unsigned short promotes to int)
        unsigned short us_val = (unsigned short)(i % 256);
        if (us_val <= (unsigned short)(a[i] % 256)) {
            le_result[i] = us_val + 1000;  // Offset result
        } else if (us_val <= 128) {        // Comparison with constant
            le_result[i] = 999;
        } else {
            le_result[i] = 0;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification: count true comparisons
    int gt_count = 0, ge_count = 0, lt_count = 0, le_count = 0;
    for (int i = 0; i < N; ++i) {
        if (gt_result[i] > 0) gt_count++;
        if (ge_result[i] != 0) ge_count++;
        if (lt_result[i] > 0) lt_count++;
        if (le_result[i] > 0) le_count++;
    }
    
    printf("GT true: %d, GE true: %d, LT true: %d, LE true: %d\n",
           gt_count, ge_count, lt_count, le_count);
    
    return 0;
}
