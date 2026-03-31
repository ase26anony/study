#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Initialize source arrays with non-uniform data
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Seed RNG for reproducibility
    srand(42);
    
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = rand() % 1000;
        d[i] = rand() % 1000;
        e[i] = rand() % 1000;
    }
    
    // Destination arrays for each comparison type
    int gt_result[N];
    int ge_result[N];
    int lt_result[N];
    int le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    const int GT_CONST = 500;
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        int cmp1 = (a[i] > b[i]) ? 1 : 0;
        // Array-to-constant comparison
        int cmp2 = (c[i] > GT_CONST) ? 2 : 0;
        // Combine results with observable side effect
        gt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    const int GE_CONST = 300;
    for (int i = 0; i < N; ++i) {
        // Different integer type (unsigned short)
        int cmp1 = (d[i] >= b[i]) ? 3 : 0;
        // Array-to-constant comparison
        int cmp2 = (a[i] >= GE_CONST) ? 4 : 0;
        ge_result[i] = cmp1 + cmp2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    const int LT_CONST = 700;
    for (int i = 0; i < N; ++i) {
        // Different integer type (long)
        int cmp1 = (e[i] < c[i]) ? 5 : 0;
        // Array-to-constant comparison
        int cmp2 = (b[i] < LT_CONST) ? 6 : 0;
        lt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    const int LE_CONST = 800;
    for (int i = 0; i < N; ++i) {
        // Mix of different comparisons
        int cmp1 = (a[i] <= b[i]) ? 7 : 0;
        int cmp2 = (d[i] <= LE_CONST) ? 8 : 0;
        le_result[i] = cmp1 + cmp2;
    }
    
    // Additional loops with different patterns to increase coverage
    
    // Loop 5: Mixed GT_EXPR with different data types
    int mixed_gt[N];
    for (int i = 0; i < N; ++i) {
        // Comparison with swapped operands
        mixed_gt[i] = (b[i] > a[i]) ? 9 : 0;
    }
    
    // Loop 6: Mixed LE_EXPR with loop-invariant on left side
    int mixed_le[N];
    const int LEFT_CONST = 400;
    for (int i = 0; i < N; ++i) {
        // Constant on left side, array element on right
        mixed_le[i] = (LEFT_CONST <= c[i]) ? 10 : 0;
    }
    
    // Compute checksum to ensure all computations are used
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += mixed_gt[i];
        checksum += mixed_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
