#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Initialize source arrays with non-uniform data
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Initialize with reproducible pseudo-random data
    srand(42);
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = (i * 3) % 1000;  // Different pattern
        d[i] = (unsigned short)((i * 7) % 65535);
        e[i] = (long)((i * 11) % 10000);
    }
    
    // Destination arrays for each comparison type
    int gt_result[N];
    int ge_result[N];
    int lt_result[N];
    int le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    const int GT_THRESHOLD = 500;
    for (int i = 0; i < N; i++) {
        // Array element comparison
        int cmp1 = (a[i] > b[i]) ? 1 : 0;
        // Array-to-constant comparison
        int cmp2 = (c[i] > GT_THRESHOLD) ? 2 : 0;
        // Combine results with observable side effect
        gt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    const int GE_THRESHOLD = 300;
    for (int i = 0; i < N; i++) {
        // Different data types to test various integer comparisons
        int cmp1 = (a[i] >= b[i]) ? 3 : 0;
        // Using unsigned short type
        int cmp2 = (d[i] >= GE_THRESHOLD) ? 4 : 0;
        // Store combined result
        ge_result[i] = cmp1 + cmp2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    const int LT_THRESHOLD = 700;
    for (int i = 0; i < N; i++) {
        // Array element comparison (will trigger std::swap in the uncovered code)
        int cmp1 = (b[i] < a[i]) ? 5 : 0;
        // Array-to-constant comparison
        int cmp2 = (c[i] < LT_THRESHOLD) ? 6 : 0;
        // Store result
        lt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    const int LE_THRESHOLD = 800;
    for (int i = 0; i < N; i++) {
        // Using long data type
        int cmp1 = (e[i] <= (long)LE_THRESHOLD) ? 7 : 0;
        // Another array-to-array comparison
        int cmp2 = (d[i] <= (unsigned short)c[i]) ? 8 : 0;
        // Store result
        le_result[i] = cmp1 + cmp2;
    }
    
    // Additional loops with different patterns to increase coverage probability
    
    // Loop 5: Mixed comparisons in same loop (if supported)
    int mixed_result[N];
    for (int i = 0; i < N; i++) {
        int val = 0;
        val += (a[i] > 100) ? 1 : 0;    // GT_EXPR
        val += (b[i] >= 200) ? 2 : 0;   // GE_EXPR
        val += (c[i] < 900) ? 4 : 0;    // LT_EXPR
        val += (d[i] <= 400) ? 8 : 0;   // LE_EXPR
        mixed_result[i] = val;
    }
    
    // Loop 6: Direct conditional assignment without ternary
    int direct_result[N];
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            direct_result[i] = a[i];
        } else {
            direct_result[i] = b[i];
        }
    }
    
    // Compute checksum to ensure all computations are used
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += mixed_result[i];
        checksum += direct_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification: count true comparisons
    int gt_count = 0, ge_count = 0, lt_count = 0, le_count = 0;
    for (int i = 0; i < N; i++) {
        gt_count += (a[i] > b[i]) ? 1 : 0;
        ge_count += (a[i] >= b[i]) ? 1 : 0;
        lt_count += (a[i] < b[i]) ? 1 : 0;
        le_count += (a[i] <= b[i]) ? 1 : 0;
    }
    
    printf("GT: %d, GE: %d, LT: %d, LE: %d\n", gt_count, ge_count, lt_count, le_count);
    
    return 0;
}
