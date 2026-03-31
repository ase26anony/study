#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare source arrays with distinct data
    int a[N], b[N], c[N];
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize arrays with reproducible pseudo-random data
    srand(42);
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = rand() % 1000;
    }
    
    // Define loop-invariant constants for comparison
    const int CONST_LIMIT = 500;
    const int VAL1 = 1;
    const int VAL2 = 0;
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (a[i] > b[i]) {
            gt_result[i] = VAL1;
        } else {
            gt_result[i] = VAL2;
        }
        
        // Array-to-constant comparison (additional to ensure coverage)
        if (b[i] > CONST_LIMIT) {
            gt_result[i] += VAL1;  // Modify result to prevent dead code elimination
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (a[i] >= c[i]) {
            ge_result[i] = VAL1;
        } else {
            ge_result[i] = VAL2;
        }
        
        // Array-to-constant comparison
        if (c[i] >= CONST_LIMIT) {
            ge_result[i] += VAL1;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (b[i] < c[i]) {
            lt_result[i] = VAL1;
        } else {
            lt_result[i] = VAL2;
        }
        
        // Array-to-constant comparison
        if (a[i] < CONST_LIMIT) {
            lt_result[i] += VAL1;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (c[i] <= a[i]) {
            le_result[i] = VAL1;
        } else {
            le_result[i] = VAL2;
        }
        
        // Array-to-constant comparison
        if (b[i] <= CONST_LIMIT) {
            le_result[i] += VAL1;
        }
    }
    
    // Compute checksum to ensure all loops have observable effects
    unsigned long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    // Additional verification: print first few results
    printf("First 5 results:\n");
    for (int i = 0; i < 5 && i < N; ++i) {
        printf("  [%d]: gt=%d, ge=%d, lt=%d, le=%d\n", 
               i, gt_result[i], ge_result[i], lt_result[i], le_result[i]);
    }
    
    return 0;
}
