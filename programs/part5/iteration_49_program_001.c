#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays with distinct patterns
    int a[N], b[N], c[N];
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 3) % 100;          // Pattern 1: 0, 3, 6, 9, ...
        b[i] = (i * 5) % 100;          // Pattern 2: 0, 5, 10, 15, ...
        c[i] = (i * 7) % 100;          // Pattern 3: 0, 7, 14, 21, ...
    }
    
    // Destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            gt_result[i] = 0;
        }
        // Also include array-to-constant comparison
        if (c[i] > 50) {
            gt_result[i] += 2;  // Add 2 if c[i] > 50
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        if (a[i] >= c[i]) {
            ge_result[i] = 1;
        } else {
            ge_result[i] = 0;
        }
        // Array-to-constant comparison
        if (b[i] >= 75) {
            ge_result[i] += 4;  // Add 4 if b[i] >= 75
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        if (b[i] < a[i]) {
            lt_result[i] = 1;
        } else {
            lt_result[i] = 0;
        }
        // Array-to-constant comparison
        if (c[i] < 25) {
            lt_result[i] += 8;  // Add 8 if c[i] < 25
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        if (c[i] <= b[i]) {
            le_result[i] = 1;
        } else {
            le_result[i] = 0;
        }
        // Array-to-constant comparison
        if (a[i] <= 10) {
            le_result[i] += 16;  // Add 16 if a[i] <= 10
        }
    }
    
    // Compute checksum to prevent dead code elimination
    unsigned long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    // Additional verification: count true comparisons
    int gt_count = 0, ge_count = 0, lt_count = 0, le_count = 0;
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) gt_count++;
        if (a[i] >= c[i]) ge_count++;
        if (b[i] < a[i]) lt_count++;
        if (c[i] <= b[i]) le_count++;
    }
    
    printf("GT: %d, GE: %d, LT: %d, LE: %d\n", 
           gt_count, ge_count, lt_count, le_count);
    
    return 0;
}
