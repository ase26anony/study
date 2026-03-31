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
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = i % 256;  // Simple pattern
        d[i] = (unsigned short)(i * 3 % 65535);
        e[i] = (long)i * 1000 - 500;
    }
    
    // Destination arrays for results
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR
    for (int i = 0; i < N; ++i) {
        // Compare array elements with each other
        gt_result[i] = (a[i] > b[i]) ? 1 : 0;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR
    for (int i = 0; i < N; ++i) {
        // Compare array element with constant
        ge_result[i] = (c[i] >= 128) ? 1 : 0;
    }
    
    // Loop 3: Less-than (LT_EXPR) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR + swap
    for (int i = 0; i < N; ++i) {
        // Compare array elements with each other (different types)
        lt_result[i] = (d[i] < (unsigned short)(i * 2 % 65535)) ? 1 : 0;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR + swap
    for (int i = 0; i < N; ++i) {
        // Compare array element with constant
        le_result[i] = (e[i] <= 500000) ? 1 : 0;
    }
    
    // Additional loops with mixed comparisons to increase coverage
    
    // Loop 5: GT_EXPR with different integer type
    int gt_result2[N];
    for (int i = 0; i < N; ++i) {
        gt_result2[i] = (e[i] > 0) ? 1 : 0;
    }
    
    // Loop 6: GE_EXPR with array-to-array comparison
    int ge_result2[N];
    for (int i = 0; i < N; ++i) {
        ge_result2[i] = (b[i] >= a[i]) ? 1 : 0;
    }
    
    // Loop 7: LT_EXPR with constant comparison
    int lt_result2[N];
    for (int i = 0; i < N; ++i) {
        lt_result2[i] = (a[i] < 500) ? 1 : 0;
    }
    
    // Loop 8: LE_EXPR with array-to-array comparison
    int le_result2[N];
    for (int i = 0; i < N; ++i) {
        le_result2[i] = (c[i] <= d[i]) ? 1 : 0;
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
        checksum += gt_result2[i] + ge_result2[i] + lt_result2[i] + le_result2[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
