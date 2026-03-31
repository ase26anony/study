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
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;           // 0-999
        b[i] = (i * 3) % 1000;          // Pattern based on index
        c[i] = 800 - (i % 400);         // Another pattern
    }
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            // Array-to-constant comparison
            gt_result[i] = (a[i] > CONST_LIMIT) ? 2 : 0;
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    // Different arrays to avoid dependencies
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        if (b[i] >= c[i]) {
            ge_result[i] = 3;
        } else {
            // Array-to-constant comparison
            ge_result[i] = (b[i] >= CONST_LIMIT) ? 4 : 0;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Using different operand order to test std::swap logic
    for (int i = 0; i < N; ++i) {
        // Note: a[i] < b[i] should trigger the swap logic
        if (a[i] < b[i]) {
            lt_result[i] = 5;
        } else {
            // Constant comparison
            lt_result[i] = (a[i] < CONST_LIMIT) ? 6 : 0;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        if (c[i] <= b[i]) {
            le_result[i] = 7;
        } else {
            // Array-to-constant comparison
            le_result[i] = (c[i] <= CONST_LIMIT) ? 8 : 0;
        }
    }
    
    // Additional loops with different integer types to increase coverage
    unsigned short us_a[N], us_b[N];
    int us_result[N];
    
    for (int i = 0; i < N; ++i) {
        us_a[i] = (unsigned short)(a[i] % 65535);
        us_b[i] = (unsigned short)(b[i] % 65535);
    }
    
    // Loop with unsigned short comparisons
    for (int i = 0; i < N; ++i) {
        // Mix of comparison types with different types
        if (us_a[i] > us_b[i]) {
            us_result[i] = 9;
        } else if (us_a[i] <= CONST_LIMIT) {
            us_result[i] = 10;
        } else {
            us_result[i] = 0;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    uint64_t checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i] + us_result[i];
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    // Additional verification print to ensure all loops execute
    printf("Sample results at index 0: gt=%d, ge=%d, lt=%d, le=%d, us=%d\n",
           gt_result[0], ge_result[0], lt_result[0], le_result[0], us_result[0]);
    
    return 0;
}
