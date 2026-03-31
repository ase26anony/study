#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare and initialize source arrays with distinct data
    int a[N], b[N];
    unsigned short c[N];
    long d[N];
    
    // Initialize with reproducible patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        c[i] = (unsigned short)((i * 5) % 256);
        d[i] = (long)((i * 11) % 200);
    }
    
    // Destination arrays for each comparison type
    int gt_result[N];
    int ge_result[N];
    int lt_result[N];
    int le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    const int GT_THRESHOLD = 50;
    for (int i = 0; i < N; i++) {
        // Array element comparison
        int cmp1 = (a[i] > b[i]) ? 1 : 0;
        // Array element vs constant comparison
        int cmp2 = (c[i] > GT_THRESHOLD) ? 2 : 0;
        // Store combined result
        gt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    const int GE_THRESHOLD = 75;
    for (int i = 0; i < N; i++) {
        // Array element comparison
        int cmp1 = (a[i] >= b[i]) ? 1 : 0;
        // Array element vs constant comparison
        int cmp2 = (d[i] >= GE_THRESHOLD) ? 2 : 0;
        // Store combined result
        ge_result[i] = cmp1 + cmp2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    const int LT_THRESHOLD = 25;
    for (int i = 0; i < N; i++) {
        // Array element comparison
        int cmp1 = (b[i] < a[i]) ? 1 : 0;
        // Array element vs constant comparison
        int cmp2 = (c[i] < LT_THRESHOLD) ? 2 : 0;
        // Store combined result
        lt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    const int LE_THRESHOLD = 150;
    for (int i = 0; i < N; i++) {
        // Array element comparison
        int cmp1 = (d[i] <= b[i]) ? 1 : 0;
        // Array element vs constant comparison
        int cmp2 = (a[i] <= LE_THRESHOLD) ? 2 : 0;
        // Store combined result
        le_result[i] = cmp1 + cmp2;
    }
    
    // Compute checksum to prevent dead code elimination
    long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
