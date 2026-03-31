#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Initialize source arrays with distinct patterns
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Initialize with reproducible pseudo-random values
    srand(42);
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = rand() % 1000;
        d[i] = rand() % 65535;
        e[i] = rand() % 10000;
    }
    
    // Destination arrays for results
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        int cmp1 = (a[i] > b[i]) ? 1 : 0;
        // Array element vs constant comparison
        int cmp2 = (c[i] > 500) ? 2 : 0;
        // Combine results with observable side-effect
        gt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    // Using different data types and comparison patterns
    for (int i = 0; i < N; ++i) {
        // Unsigned short comparison (different integer type)
        int cmp1 = (d[i] >= 32768) ? 3 : 0;
        // Array-to-array comparison with different arrays
        int cmp2 = (b[i] >= c[i]) ? 1 : 0;
        // Store combined result
        ge_result[i] = cmp1 + cmp2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Mix comparisons to trigger std::swap(cond_expr0, cond_expr1)
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        int cmp1 = (a[i] < b[i]) ? 1 : 0;
        // Array element vs constant (different constant)
        int cmp2 = (e[i] < 5000) ? 4 : 0;
        // Store with observable effect
        lt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    // Using long data type and mixed comparisons
    for (int i = 0; i < N; ++i) {
        // Long integer comparison
        int cmp1 = (e[i] <= 2500) ? 2 : 0;
        // Array-to-array with different operand order
        int cmp2 = (c[i] <= a[i]) ? 1 : 0;
        // Store result
        le_result[i] = cmp1 + cmp2;
    }
    
    // Additional loops with pure array-to-array comparisons
    // to ensure all transformation paths are exercised
    int pure_gt[N], pure_ge[N], pure_lt[N], pure_le[N];
    
    // Pure GT comparisons
    for (int i = 0; i < N; ++i) {
        pure_gt[i] = (a[i] > c[i]) ? 1 : 0;
    }
    
    // Pure GE comparisons  
    for (int i = 0; i < N; ++i) {
        pure_ge[i] = (b[i] >= d[i]) ? 1 : 0;
    }
    
    // Pure LT comparisons
    for (int i = 0; i < N; ++i) {
        pure_lt[i] = (c[i] < e[i]) ? 1 : 0;
    }
    
    // Pure LE comparisons
    for (int i = 0; i < N; ++i) {
        pure_le[i] = (d[i] <= e[i]) ? 1 : 0;
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
        checksum += pure_gt[i] + pure_ge[i] + pure_lt[i] + pure_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
