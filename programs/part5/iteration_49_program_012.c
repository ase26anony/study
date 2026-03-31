#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays with non-uniform data
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
        // Compare array elements with each other
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            gt_result[i] = 0;
        }
        // Also compare with loop-invariant constant
        if (c[i] > 50) {
            gt_result[i] += 2;
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Compare array elements with each other
        if (a[i] >= c[i]) {
            ge_result[i] = 1;
        } else {
            ge_result[i] = 0;
        }
        // Also compare with loop-invariant constant
        if (b[i] >= 25) {
            ge_result[i] += 2;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Compare array elements with each other
        if (b[i] < a[i]) {
            lt_result[i] = 1;
        } else {
            lt_result[i] = 0;
        }
        // Also compare with loop-invariant constant
        if (c[i] < 75) {
            lt_result[i] += 2;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Compare array elements with each other
        if (c[i] <= b[i]) {
            le_result[i] = 1;
        } else {
            le_result[i] = 0;
        }
        // Also compare with loop-invariant constant
        if (a[i] <= 60) {
            le_result[i] += 2;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    unsigned long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
