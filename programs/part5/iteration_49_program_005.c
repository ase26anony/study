#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays
    int a[N], b[N];
    int c[N], d[N];
    
    // Initialize with reproducible pseudo-random values
    srand(42);
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = rand() % 1000;
        d[i] = rand() % 1000;
    }
    
    // Destination arrays for results
    int gt_result[N];
    int ge_result[N];
    int lt_result[N];
    int le_result[N];
    
    // Loop 1: Greater-than comparisons (GT_EXPR)
    for (int i = 0; i < N; ++i) {
        // Mix of array-to-array and array-to-constant comparisons
        if (a[i] > b[i]) {
            gt_result[i] = 1;  // True case
        } else {
            gt_result[i] = 0;  // False case
        }
    }
    
    // Loop 2: Greater-than-or-equal comparisons (GE_EXPR)
    const int GE_LIMIT = 500;
    for (int i = 0; i < N; ++i) {
        // Array-to-constant comparison
        if (a[i] >= GE_LIMIT) {
            ge_result[i] = a[i];  // Use actual value when true
        } else {
            ge_result[i] = 0;     // Zero when false
        }
    }
    
    // Loop 3: Less-than comparisons (LT_EXPR)
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (c[i] < d[i]) {
            lt_result[i] = c[i] + d[i];  // Sum when true
        } else {
            lt_result[i] = c[i] - d[i];  // Difference when false
        }
    }
    
    // Loop 4: Less-than-or-equal comparisons (LE_EXPR)
    const int LE_LIMIT = 750;
    for (int i = 0; i < N; ++i) {
        // Array-to-constant comparison
        if (b[i] <= LE_LIMIT) {
            le_result[i] = b[i] * 2;  // Double when true
        } else {
            le_result[i] = b[i] / 2;  // Half when false
        }
    }
    
    // Additional loop with mixed comparisons to ensure coverage
    int mixed_result[N];
    for (int i = 0; i < N; ++i) {
        // Mix of all four comparison types in one loop
        int val = 0;
        if (a[i] > b[i]) val += 1;
        if (a[i] >= c[i]) val += 2;
        if (b[i] < d[i]) val += 4;
        if (c[i] <= LE_LIMIT) val += 8;
        mixed_result[i] = val;
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += mixed_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
