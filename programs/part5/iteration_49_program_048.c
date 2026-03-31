#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare source arrays with different data patterns
    int a[N], b[N], c[N];
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize source arrays with reproducible pseudo-random data
    srand(42);
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = 500 + (i % 100);  // Values between 500-599
    }
    
    // Loop 1: Greater-than comparisons (GT_EXPR)
    for (int i = 0; i < N; i++) {
        // Compare array elements (varying RHS)
        gt_result[i] = (a[i] > b[i]) ? 1 : 0;
        // Also compare with constant (invariant RHS) to test both paths
        gt_result[i] += (a[i] > 500) ? 2 : 0;
    }
    
    // Loop 2: Greater-than-or-equal comparisons (GE_EXPR)
    for (int i = 0; i < N; i++) {
        // Compare array elements
        ge_result[i] = (a[i] >= b[i]) ? 1 : 0;
        // Compare with constant
        ge_result[i] += (c[i] >= 550) ? 2 : 0;
    }
    
    // Loop 3: Less-than comparisons (LT_EXPR)
    for (int i = 0; i < N; i++) {
        // Compare array elements
        lt_result[i] = (a[i] < b[i]) ? 1 : 0;
        // Compare with constant
        lt_result[i] += (b[i] < 250) ? 2 : 0;
    }
    
    // Loop 4: Less-than-or-equal comparisons (LE_EXPR)
    for (int i = 0; i < N; i++) {
        // Compare array elements
        le_result[i] = (a[i] <= b[i]) ? 1 : 0;
        // Compare with constant
        le_result[i] += (c[i] <= 580) ? 2 : 0;
    }
    
    // Additional loops with different integer types to increase coverage
    unsigned short us_a[N], us_b[N];
    unsigned short us_result[N];
    
    for (int i = 0; i < N; i++) {
        us_a[i] = (unsigned short)(rand() % 65535);
        us_b[i] = (unsigned short)(rand() % 65535);
    }
    
    // Loop 5: Greater-than with unsigned short
    for (int i = 0; i < N; i++) {
        us_result[i] = (us_a[i] > us_b[i]) ? 1 : 0;
    }
    
    // Loop 6: Less-than-or-equal with unsigned short
    for (int i = 0; i < N; i++) {
        us_result[i] += (us_a[i] <= 32768) ? 2 : 0;
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i] + us_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
