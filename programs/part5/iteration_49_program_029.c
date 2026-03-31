#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays with distinct patterns
    int a[N], b[N], c[N];
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 3) % 100;          // Pattern 1: 0-99 repeating
        b[i] = (i * 7 + 5) % 100;      // Pattern 2: 5-104 mod 100
        c[i] = (i * 11) % 100;         // Pattern 3: different pattern
    }
    
    // Declare destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        int cond1 = (a[i] > b[i]) ? 1 : 0;
        // Array-to-constant comparison
        int cond2 = (c[i] > 50) ? 2 : 0;
        // Combine results with observable side-effect
        gt_result[i] = cond1 + cond2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        int cond1 = (a[i] >= b[i]) ? 3 : 0;
        // Array-to-constant comparison
        int cond2 = (c[i] >= 75) ? 4 : 0;
        // Combine results
        ge_result[i] = cond1 + cond2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        int cond1 = (a[i] < b[i]) ? 5 : 0;
        // Array-to-constant comparison
        int cond2 = (c[i] < 25) ? 6 : 0;
        // Combine results
        lt_result[i] = cond1 + cond2;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        int cond1 = (a[i] <= b[i]) ? 7 : 0;
        // Array-to-constant comparison
        int cond2 = (c[i] <= 60) ? 8 : 0;
        // Combine results
        le_result[i] = cond1 + cond2;
    }
    
    // Additional loops with different integer types to increase coverage
    unsigned short us_a[N], us_b[N];
    unsigned short us_result[N];
    for (int i = 0; i < N; ++i) {
        us_a[i] = (i * 13) % 256;
        us_b[i] = (i * 17 + 10) % 256;
    }
    
    // Loop 5: Greater-than with unsigned short
    for (int i = 0; i < N; ++i) {
        us_result[i] = (us_a[i] > us_b[i]) ? 100 : 200;
    }
    
    // Loop 6: Less-than-or-equal with unsigned short
    for (int i = 0; i < N; ++i) {
        us_result[i] = (us_a[i] <= us_b[i]) ? 300 : 400;
    }
    
    // Compute checksum to ensure all loops have observable effects
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i] + us_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional test with long type
    long long la[N], lb[N];
    long long l_result[N];
    for (int i = 0; i < N; ++i) {
        la[i] = (long long)i * 1000;
        lb[i] = (long long)i * 1000 + 500;
    }
    
    // Loop 7: Greater-than-or-equal with long long
    for (int i = 0; i < N; ++i) {
        l_result[i] = (la[i] >= lb[i]) ? -1 : -2;
        checksum += l_result[i];
    }
    
    // Loop 8: Less-than with long long
    for (int i = 0; i < N; ++i) {
        l_result[i] = (la[i] < lb[i]) ? -3 : -4;
        checksum += l_result[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}
