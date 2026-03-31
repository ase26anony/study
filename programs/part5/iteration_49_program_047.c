#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare source arrays with different data patterns
    int a[N], b[N], c[N];
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize with reproducible pseudo-random data
    srand(42);
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = (i * 3) % 500;  // Different pattern for constant comparisons
    }
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            gt_result[i] = 0;
        }
        
        // Also include array-to-constant comparison
        if (a[i] > 500) {
            gt_result[i] += 2;  // Add different value for constant comparison
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (a[i] >= b[i]) {
            ge_result[i] = 1;
        } else {
            ge_result[i] = 0;
        }
        
        // Array-to-constant comparison
        if (b[i] >= 250) {
            ge_result[i] += 2;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Use different arrays to avoid dependencies
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (c[i] < a[i]) {
            lt_result[i] = 1;
        } else {
            lt_result[i] = 0;
        }
        
        // Array-to-constant comparison
        if (c[i] < 300) {
            lt_result[i] += 2;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (b[i] <= c[i]) {
            le_result[i] = 1;
        } else {
            le_result[i] = 0;
        }
        
        // Array-to-constant comparison
        if (a[i] <= 750) {
            le_result[i] += 2;
        }
    }
    
    // Additional loops with different integer types to increase coverage
    unsigned short us_a[N], us_b[N];
    unsigned short us_result[N];
    
    for (int i = 0; i < N; ++i) {
        us_a[i] = (unsigned short)(a[i] % 65535);
        us_b[i] = (unsigned short)(b[i] % 65535);
    }
    
    // Loop with unsigned short comparisons
    for (int i = 0; i < N; ++i) {
        // Mix of comparison types with unsigned types
        if (us_a[i] > us_b[i]) {
            us_result[i] = 1;
        } else if (us_a[i] <= 32768) {
            us_result[i] = 2;
        } else {
            us_result[i] = 0;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i] + us_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification: count true comparisons
    int gt_count = 0, ge_count = 0, lt_count = 0, le_count = 0;
    for (int i = 0; i < N; ++i) {
        gt_count += (a[i] > b[i]);
        ge_count += (a[i] >= b[i]);
        lt_count += (c[i] < a[i]);
        le_count += (b[i] <= c[i]);
    }
    
    printf("GT: %d, GE: %d, LT: %d, LE: %d\n", gt_count, ge_count, lt_count, le_count);
    
    return 0;
}
