#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define CONST_LIMIT 500

int main() {
    // Initialize source arrays with non-uniform data
    int a[N], b[N], c[N];
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Seed for reproducibility
    srand(42);
    
    // Initialize arrays with varying data
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;          // 0-999
        b[i] = (i * 3) % 1000;         // Pattern based on index
        c[i] = 800 - (i % 400);        // Another pattern
    }
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison (a[i] > b[i])
        // This should trigger GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR transformation
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            gt_result[i] = 0;
        }
        
        // Additional comparison with constant to test different RHS
        if (a[i] > CONST_LIMIT) {
            gt_result[i] += 10;  // Modify result based on constant comparison
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    // Use different arrays to avoid dependencies
    for (int i = 0; i < N; i++) {
        // Array element comparison (b[i] >= c[i])
        // This should trigger GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR transformation
        if (b[i] >= c[i]) {
            ge_result[i] = 2;
        } else {
            ge_result[i] = -1;
        }
        
        // Comparison with loop-invariant constant
        if (b[i] >= CONST_LIMIT) {
            ge_result[i] *= 3;  // Modify based on constant comparison
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Use different operand order to test std::swap logic
    for (int i = 0; i < N; i++) {
        // Array element comparison (c[i] < a[i])
        // This should trigger LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR + swap
        if (c[i] < a[i]) {
            lt_result[i] = a[i];
        } else {
            lt_result[i] = c[i];
        }
        
        // Comparison with constant
        if (c[i] < CONST_LIMIT) {
            lt_result[i] += 1000;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    // Mix different data types to ensure integer comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison (a[i] <= b[i])
        // This should trigger LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR + swap
        if (a[i] <= b[i]) {
            le_result[i] = b[i] - a[i];
        } else {
            le_result[i] = a[i] - b[i];
        }
        
        // Multiple comparisons to increase vectorization opportunities
        if (a[i] <= CONST_LIMIT) {
            le_result[i] *= 2;
        }
        if (b[i] <= CONST_LIMIT + 200) {
            le_result[i] += 50;
        }
    }
    
    // Compute checksum to ensure all loops have observable effects
    uint64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    
    // Additional verification: print first few results
    printf("First 5 results:\n");
    for (int i = 0; i < 5 && i < N; i++) {
        printf("i=%d: gt=%d, ge=%d, lt=%d, le=%d\n",
               i, gt_result[i], ge_result[i], lt_result[i], le_result[i]);
    }
    
    return 0;
}
