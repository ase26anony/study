#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays with distinct data
    int a[N], b[N], c[N];
    for (int i = 0; i < N; ++i) {
        a[i] = i * 3 + 1;
        b[i] = i * 2 + 5;
        c[i] = i * 4 - 3;
    }
    
    // Destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            gt_result[i] = 0;
        }
        // Also include array-to-constant comparison
        if (c[i] > 500) {
            gt_result[i] += 2;  // Add to existing value
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (a[i] >= c[i]) {
            ge_result[i] = 1;
        } else {
            ge_result[i] = 0;
        }
        // Array-to-constant comparison
        if (b[i] >= 1000) {
            ge_result[i] += 3;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (b[i] < a[i]) {
            lt_result[i] = 1;
        } else {
            lt_result[i] = 0;
        }
        // Array-to-constant comparison
        if (c[i] < 2000) {
            lt_result[i] += 4;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (c[i] <= b[i]) {
            le_result[i] = 1;
        } else {
            le_result[i] = 0;
        }
        // Array-to-constant comparison
        if (a[i] <= 1500) {
            le_result[i] += 5;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    unsigned long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    // Additional verification: print first few results
    printf("First 5 results:\n");
    for (int i = 0; i < 5 && i < N; ++i) {
        printf("i=%d: gt=%d, ge=%d, lt=%d, le=%d\n",
               i, gt_result[i], ge_result[i], lt_result[i], le_result[i]);
    }
    
    return 0;
}
