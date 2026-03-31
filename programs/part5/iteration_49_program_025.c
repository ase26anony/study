#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Initialize source arrays with non-uniform data
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Seed for reproducibility
    srand(42);
    
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 3) % 100;          // Pattern: 0, 3, 6, 9, ...
        b[i] = (i * 5) % 100;          // Pattern: 0, 5, 10, 15, ...
        c[i] = rand() % 200 - 100;     // Random values between -100 and 99
        d[i] = (unsigned short)(i * 7) % 256;  // Pattern for unsigned short
        e[i] = (long)i * 2;            // Pattern: 0, 2, 4, 6, ...
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
        
        // Additional comparison with constant to test both RHS types
        if (c[i] > 50) {
            gt_result[i] += 10;  // Modify result based on constant comparison
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    // Using different data types and comparison patterns
    for (int i = 0; i < N; ++i) {
        // Comparison between array elements of different types
        if ((int)d[i] >= a[i]) {
            ge_result[i] = d[i];  // Store the unsigned short value
        } else {
            ge_result[i] = -1;
        }
        
        // Comparison with loop-invariant constant
        if (b[i] >= 25) {
            ge_result[i] += 100;  // Add constant if condition met
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Note: This should trigger std::swap(cond_expr0, cond_expr1)
    for (int i = 0; i < N; ++i) {
        // Direct array element comparison
        if (a[i] < b[i]) {
            lt_result[i] = a[i];  // Store smaller value
        } else {
            lt_result[i] = b[i];  // Store other value
        }
        
        // Comparison with negative constant
        if (c[i] < -25) {
            lt_result[i] = -lt_result[i];  // Negate if condition met
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    // Using long data type for variety
    for (int i = 0; i < N; ++i) {
        // Comparison between long values
        if (e[i] <= (long)a[i] * 2) {
            le_result[i] = (int)e[i];  // Store converted value
        } else {
            le_result[i] = 999;
        }
        
        // Multiple comparisons with different constants
        if (b[i] <= 75) {
            le_result[i] += 50;
        }
        if (d[i] <= 128) {
            le_result[i] += 25;
        }
    }
    
    // Compute checksum to ensure all loops have observable effects
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification: count how many comparisons were true
    int gt_count = 0, ge_count = 0, lt_count = 0, le_count = 0;
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) gt_count++;
        if ((int)d[i] >= a[i]) ge_count++;
        if (a[i] < b[i]) lt_count++;
        if (e[i] <= (long)a[i] * 2) le_count++;
    }
    
    printf("True comparisons: GT=%d, GE=%d, LT=%d, LE=%d\n", 
           gt_count, ge_count, lt_count, le_count);
    
    return 0;
}
