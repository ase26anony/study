#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays with distinct patterns
    int a[N], b[N], c[N];
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 3) % 100;          // Pattern 1: 0, 3, 6, 9, ...
        b[i] = (i * 5) % 100;          // Pattern 2: 0, 5, 10, 15, ...
        c[i] = (i * 7) % 100;          // Pattern 3: 0, 7, 14, 21, ...
    }
    
    // Destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        int cond1 = (a[i] > b[i]);
        // Array-to-constant comparison
        int cond2 = (c[i] > 50);
        // Combine results with bitwise OR to ensure both are used
        gt_result[i] = (cond1 | cond2) ? 1 : 0;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        int cond1 = (a[i] >= b[i]);
        // Array-to-constant comparison
        int cond2 = (c[i] >= 30);
        // Combine results with bitwise AND to ensure both are used
        ge_result[i] = (cond1 & cond2) ? 2 : 0;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        int cond1 = (a[i] < b[i]);
        // Array-to-constant comparison
        int cond2 = (c[i] < 70);
        // Combine results with bitwise OR to ensure both are used
        lt_result[i] = (cond1 | cond2) ? 3 : 0;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        int cond1 = (a[i] <= b[i]);
        // Array-to-constant comparison
        int cond2 = (c[i] <= 90);
        // Combine results with bitwise AND to ensure both are used
        le_result[i] = (cond1 & cond2) ? 4 : 0;
    }
    
    // Additional loops with different integer types to increase coverage
    unsigned short us_a[N], us_b[N];
    unsigned short us_result[N];
    for (int i = 0; i < N; ++i) {
        us_a[i] = (i * 11) % 256;
        us_b[i] = (i * 13) % 256;
    }
    
    // Loop 5: Greater-than with unsigned short
    for (int i = 0; i < N; ++i) {
        us_result[i] = (us_a[i] > us_b[i]) ? 5 : 0;
    }
    
    // Loop 6: Less-than-or-equal with unsigned short
    for (int i = 0; i < N; ++i) {
        us_result[i] = (us_a[i] <= us_b[i]) ? 6 : 0;
    }
    
    // Compute checksum to ensure all loops have observable effects
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i] + us_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional test with long type
    long long la[N], lb[N], lresult[N];
    for (int i = 0; i < N; ++i) {
        la[i] = (long long)i * 1000;
        lb[i] = (long long)i * 1000 + 500;
    }
    
    // Loop 7: Greater-than-or-equal with long long
    for (int i = 0; i < N; ++i) {
        lresult[i] = (la[i] >= lb[i]) ? 7 : 0;
        checksum += lresult[i];
    }
    
    // Loop 8: Less-than with long long
    for (int i = 0; i < N; ++i) {
        lresult[i] = (la[i] < lb[i]) ? 8 : 0;
        checksum += lresult[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}
