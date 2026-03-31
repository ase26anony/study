#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Initialize source arrays with distinct patterns
    int a[N], b[N];
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Seed for reproducibility
    srand(42);
    
    // Initialize arrays with non-uniform data
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = (i * 3) % 1000;
    }
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            gt_result[i] = 0;
        }
        
        // Additional constant comparison to test both patterns
        if (a[i] > 500) {
            gt_result[i] += 2;  // Add 2 if greater than constant
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (a[i] >= b[i]) {
            ge_result[i] = 1;
        } else {
            ge_result[i] = 0;
        }
        
        // Additional constant comparison
        if (a[i] >= 250) {
            ge_result[i] += 2;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (a[i] < b[i]) {
            lt_result[i] = 1;
        } else {
            lt_result[i] = 0;
        }
        
        // Additional constant comparison
        if (a[i] < 750) {
            lt_result[i] += 2;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (a[i] <= b[i]) {
            le_result[i] = 1;
        } else {
            le_result[i] = 0;
        }
        
        // Additional constant comparison
        if (a[i] <= 600) {
            le_result[i] += 2;
        }
    }
    
    // Compute checksum to ensure all loops have observable effects
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    // Additional verification: print first few results
    printf("First 5 results:\n");
    for (int i = 0; i < 5 && i < N; i++) {
        printf("a[%d]=%d, b[%d]=%d: gt=%d, ge=%d, lt=%d, le=%d\n",
               i, a[i], i, b[i], gt_result[i], ge_result[i], lt_result[i], le_result[i]);
    }
    
    return 0;
}
