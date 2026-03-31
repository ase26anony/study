#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024

int main() {
    // Seed random number generator for reproducibility
    srand(42);
    
    // Declare and initialize source arrays with distinct data
    int a[N], b[N], c[N];
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = i;  // Simple linear pattern
    }
    
    // Declare destination arrays for each comparison type
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
        if (c[i] > 500) {
            gt_result[i] += 2;  // Add to existing value
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        if (a[i] >= b[i]) {
            ge_result[i] = 1;
        } else {
            ge_result[i] = 0;
        }
        // Array-to-constant comparison
        if (c[i] >= 750) {
            ge_result[i] += 4;  // Different constant to avoid pattern matching
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Use different arrays to avoid data dependencies
    for (int i = 0; i < N; ++i) {
        // Array element comparison (swapped operands compared to GT)
        if (b[i] < a[i]) {
            lt_result[i] = 1;
        } else {
            lt_result[i] = 0;
        }
        // Array-to-constant comparison
        if (c[i] < 250) {
            lt_result[i] += 8;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        if (b[i] <= a[i]) {
            le_result[i] = 1;
        } else {
            le_result[i] = 0;
        }
        // Array-to-constant comparison
        if (c[i] <= 100) {
            le_result[i] += 16;
        }
    }
    
    // Compute checksum to ensure all loops have observable effects
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification: print first few results
    printf("First 5 results:\n");
    for (int i = 0; i < 5 && i < N; ++i) {
        printf("i=%d: gt=%d, ge=%d, lt=%d, le=%d\n", 
               i, gt_result[i], ge_result[i], lt_result[i], le_result[i]);
    }
    
    return 0;
}
