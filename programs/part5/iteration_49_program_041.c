#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays with distinct data
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Initialize with reproducible pseudo-random data
    srand(42);
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = i % 100;
        d[i] = (unsigned short)(rand() % 65535);
        e[i] = (long)(rand() % 10000);
    }
    
    // Destination arrays for results
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: GT_EXPR (greater-than) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR
    for (int i = 0; i < N; ++i) {
        // Compare array elements with each other
        gt_result[i] = (a[i] > b[i]) ? 1 : 0;
    }
    
    // Loop 2: GE_EXPR (greater-than-or-equal) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR
    for (int i = 0; i < N; ++i) {
        // Compare array element with constant
        ge_result[i] = (c[i] >= 50) ? 1 : 0;
    }
    
    // Loop 3: LT_EXPR (less-than) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR with swap
    for (int i = 0; i < N; ++i) {
        // Compare array elements with each other (different data type)
        lt_result[i] = (d[i] < (unsigned short)(a[i] % 65535)) ? 1 : 0;
    }
    
    // Loop 4: LE_EXPR (less-than-or-equal) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR with swap
    for (int i = 0; i < N; ++i) {
        // Compare array element with constant (different data type)
        le_result[i] = (e[i] <= 5000L) ? 1 : 0;
    }
    
    // Additional loops with mixed comparisons to increase coverage probability
    
    // Loop 5: GT_EXPR with different integer type
    int gt_result2[N];
    for (int i = 0; i < N; ++i) {
        gt_result2[i] = (b[i] > c[i]) ? 1 : 0;
    }
    
    // Loop 6: GE_EXPR with array-to-array comparison
    int ge_result2[N];
    for (int i = 0; i < N; ++i) {
        ge_result2[i] = (a[i] >= c[i]) ? 1 : 0;
    }
    
    // Loop 7: LT_EXPR with constant comparison
    int lt_result2[N];
    for (int i = 0; i < N; ++i) {
        lt_result2[i] = (a[i] < 800) ? 1 : 0;
    }
    
    // Loop 8: LE_EXPR with array-to-array comparison
    int le_result2[N];
    for (int i = 0; i < N; ++i) {
        le_result2[i] = (b[i] <= a[i]) ? 1 : 0;
    }
    
    // Compute checksum to ensure all computations are used
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
        checksum += gt_result2[i] + ge_result2[i] + lt_result2[i] + le_result2[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
