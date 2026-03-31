#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Initialize source arrays with non-uniform data
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Initialize with deterministic but non-uniform values
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 5) % 100;
        c[i] = (i * 7) % 100;
        d[i] = (unsigned short)((i * 11) % 256);
        e[i] = (long)((i * 13) % 1000);
    }
    
    // Destination arrays for results
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix array-to-array and array-to-constant comparisons
    const int GT_CONST = 50;
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        int cmp1 = (a[i] > b[i]) ? 1 : 0;
        // Array-to-constant comparison
        int cmp2 = (c[i] > GT_CONST) ? 2 : 0;
        // Combine results
        gt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    // Use different data types to test various vectorization paths
    const unsigned short GE_CONST = 128;
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison with unsigned short
        int cmp1 = (d[i] >= (unsigned short)(i % 256)) ? 1 : 0;
        // Array-to-constant comparison
        int cmp2 = (d[i] >= GE_CONST) ? 2 : 0;
        // Store result
        ge_result[i] = cmp1 + cmp2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Use long data type for wider vectorization
    const long LT_CONST = 500;
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison with long
        int cmp1 = (e[i] < (long)(i * 2)) ? 1 : 0;
        // Array-to-constant comparison
        int cmp2 = (e[i] < LT_CONST) ? 2 : 0;
        // Store result
        lt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    // Mix different comparison patterns
    const int LE_CONST = 75;
    for (int i = 0; i < N; ++i) {
        // Multiple array-to-array comparisons
        int cmp1 = (a[i] <= b[i]) ? 1 : 0;
        int cmp2 = (b[i] <= c[i]) ? 2 : 0;
        // Array-to-constant comparison
        int cmp3 = (a[i] <= LE_CONST) ? 4 : 0;
        // Combine all results
        le_result[i] = cmp1 + cmp2 + cmp3;
    }
    
    // Additional loop with mixed operators to ensure all paths are exercised
    int mixed_result[N];
    for (int i = 0; i < N; ++i) {
        // Use all four operators in the same loop
        int val = 0;
        val += (a[i] > b[i]) ? 1 : 0;    // GT_EXPR
        val += (b[i] >= c[i]) ? 2 : 0;   // GE_EXPR
        val += (c[i] < d[i]) ? 4 : 0;    // LT_EXPR
        val += (d[i] <= e[i]) ? 8 : 0;   // LE_EXPR
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
    
    // Additional verification: count true comparisons
    int gt_count = 0, ge_count = 0, lt_count = 0, le_count = 0;
    for (int i = 0; i < N; ++i) {
        gt_count += (a[i] > b[i]);
        ge_count += (d[i] >= GE_CONST);
        lt_count += (e[i] < LT_CONST);
        le_count += (a[i] <= LE_CONST);
    }
    
    printf("GT true: %d, GE true: %d, LT true: %d, LE true: %d\n",
           gt_count, ge_count, lt_count, le_count);
    
    return 0;
}
