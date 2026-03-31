#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays with non-uniform data
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Initialize with deterministic but non-uniform values
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        c[i] = (i * 11) % 100;
        d[i] = (unsigned short)((i * 13) % 256);
        e[i] = (long)((i * 17) % 1000);
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
        if (d[i] > 128) {
            gt_result[i] += 2;  // Add to existing value
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    // Different data types and comparison patterns
    for (int i = 0; i < N; ++i) {
        // Array element comparison with different types
        if (c[i] >= a[i]) {
            ge_result[i] = c[i];
        } else {
            ge_result[i] = a[i];
        }
        
        // Array-to-constant comparison
        if (e[i] >= 500L) {
            ge_result[i] += 1000;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Use different arrays to avoid dependencies
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        if (b[i] < c[i]) {
            lt_result[i] = b[i];
        } else {
            lt_result[i] = c[i];
        }
        
        // Array-to-constant comparison
        if (d[i] < 64) {
            lt_result[i] = -lt_result[i];  // Transform the value
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    // Mix of signed and unsigned comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        if (a[i] <= b[i]) {
            le_result[i] = a[i] + b[i];
        } else {
            le_result[i] = a[i] - b[i];
        }
        
        // Array-to-constant comparison with unsigned type
        if (d[i] <= 192) {
            le_result[i] *= 2;
        }
    }
    
    // Additional loops with different integer types to ensure coverage
    char char_arr1[N], char_arr2[N];
    int char_result[N];
    
    for (int i = 0; i < N; ++i) {
        char_arr1[i] = (char)(i % 128);
        char_arr2[i] = (char)((i * 5) % 128);
    }
    
    // Loop 5: Another GT_EXPR with char type
    for (int i = 0; i < N; ++i) {
        char_result[i] = (char_arr1[i] > char_arr2[i]) ? 100 : -100;
    }
    
    // Loop 6: Another LE_EXPR with char type
    for (int i = 0; i < N; ++i) {
        if (char_arr1[i] <= 64) {
            char_result[i] += 50;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += char_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
