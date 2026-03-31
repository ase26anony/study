#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays
    int a[N], b[N];
    unsigned short c[N];
    long d[N];
    
    // Initialize with deterministic but non-uniform data
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        c[i] = (unsigned short)((i * 11) % 256);
        d[i] = (long)((i * 13) % 200);
    }
    
    // Destination arrays for each comparison type
    int gt_result[N];
    int ge_result[N];
    int lt_result[N];
    int le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        int cmp1 = (a[i] > b[i]) ? 1 : 0;
        // Array element vs constant comparison
        int cmp2 = (c[i] > 128) ? 2 : 0;
        // Combine results with side-effect
        gt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison with different types
        int cmp1 = (a[i] >= b[i]) ? 3 : 0;
        // Array element vs constant comparison
        int cmp2 = (d[i] >= 100L) ? 4 : 0;
        // Combine with bitwise operation to ensure materialization
        ge_result[i] = cmp1 | cmp2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Note: This should trigger std::swap(cond_expr0, cond_expr1)
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        int cmp1 = (a[i] < b[i]) ? 5 : 0;
        // Array element vs constant comparison
        int cmp2 = (c[i] < 64) ? 6 : 0;
        // Store result with side-effect
        lt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    // Note: This should also trigger std::swap(cond_expr0, cond_expr1)
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        int cmp1 = (a[i] <= b[i]) ? 7 : 0;
        // Array element vs constant comparison
        int cmp2 = (d[i] <= 150L) ? 8 : 0;
        // Store result
        le_result[i] = cmp1 | cmp2;
    }
    
    // Additional loops with different integer types to increase coverage
    char e[N];
    unsigned int f[N];
    for (int i = 0; i < N; ++i) {
        e[i] = (char)(i % 128);
        f[i] = i * 2;
    }
    
    // Loop 5: Mixed comparisons with char type
    int mixed_result[N];
    for (int i = 0; i < N; ++i) {
        // Use all four comparison operators in one loop
        int val = 0;
        val += (e[i] > 64) ? 1 : 0;
        val += (e[i] >= 32) ? 2 : 0;
        val += (e[i] < 96) ? 4 : 0;
        val += (e[i] <= 80) ? 8 : 0;
        mixed_result[i] = val;
    }
    
    // Loop 6: Unsigned integer comparisons
    int unsigned_result[N];
    for (int i = 0; i < N; ++i) {
        // Comparisons on unsigned types
        unsigned_result[i] = (f[i] > 1000) ? 1 : 0;
        unsigned_result[i] |= (f[i] >= 500) ? 2 : 0;
        unsigned_result[i] |= (f[i] < 1500) ? 4 : 0;
        unsigned_result[i] |= (f[i] <= 2000) ? 8 : 0;
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += mixed_result[i];
        checksum += unsigned_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
