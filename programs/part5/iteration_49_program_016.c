#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Initialize source arrays with distinct patterns
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Initialize with reproducible patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 5) % 100;
        c[i] = (i * 7) % 100;
        d[i] = (unsigned short)((i * 11) % 256);
        e[i] = (long)((i * 13) % 1000);
    }
    
    // Destination arrays for each comparison type
    int gt_result[N];
    int ge_result[N];
    int lt_result[N];
    int le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix array-to-array and array-to-constant comparisons
    const int GT_THRESHOLD = 50;
    for (int i = 0; i < N; i++) {
        // Array-to-array comparison
        int cmp1 = (a[i] > b[i]) ? 1 : 0;
        // Array-to-constant comparison
        int cmp2 = (c[i] > GT_THRESHOLD) ? 2 : 0;
        // Combine results
        gt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    const int GE_THRESHOLD = 30;
    for (int i = 0; i < N; i++) {
        // Array-to-array comparison with different types
        int cmp1 = (d[i] >= (unsigned short)b[i]) ? 1 : 0;
        // Array-to-constant comparison
        int cmp2 = (a[i] >= GE_THRESHOLD) ? 2 : 0;
        // Combine results
        ge_result[i] = cmp1 + cmp2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    const int LT_THRESHOLD = 70;
    for (int i = 0; i < N; i++) {
        // Array-to-array comparison
        int cmp1 = (b[i] < c[i]) ? 1 : 0;
        // Array-to-constant comparison
        int cmp2 = (a[i] < LT_THRESHOLD) ? 2 : 0;
        // Combine results
        lt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    const long LE_THRESHOLD = 500L;
    for (int i = 0; i < N; i++) {
        // Array-to-array comparison with long type
        int cmp1 = (e[i] <= (long)c[i] * 10) ? 1 : 0;
        // Array-to-constant comparison
        int cmp2 = (e[i] <= LE_THRESHOLD) ? 2 : 0;
        // Combine results
        le_result[i] = cmp1 + cmp2;
    }
    
    // Additional loop with mixed comparisons to ensure coverage
    int mixed_result[N];
    const int MIXED_THRESHOLD = 40;
    for (int i = 0; i < N; i++) {
        // Use all four comparison types in one loop
        int val = 0;
        val += (a[i] > MIXED_THRESHOLD) ? 1 : 0;   // GT_EXPR
        val += (b[i] >= MIXED_THRESHOLD) ? 2 : 0;  // GE_EXPR
        val += (c[i] < MIXED_THRESHOLD) ? 4 : 0;   // LT_EXPR
        val += (d[i] <= MIXED_THRESHOLD) ? 8 : 0;  // LE_EXPR
        mixed_result[i] = val;
    }
    
    // Compute checksum to prevent dead code elimination
    long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += mixed_result[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    
    // Additional verification to ensure all loops are executed
    int verify_gt = 0, verify_ge = 0, verify_lt = 0, verify_le = 0;
    for (int i = 0; i < N; i++) {
        verify_gt += (gt_result[i] > 0) ? 1 : 0;
        verify_ge += (ge_result[i] > 0) ? 1 : 0;
        verify_lt += (lt_result[i] > 0) ? 1 : 0;
        verify_le += (le_result[i] > 0) ? 1 : 0;
    }
    
    printf("GT positives: %d, GE positives: %d, LT positives: %d, LE positives: %d\n",
           verify_gt, verify_ge, verify_lt, verify_le);
    
    return 0;
}
