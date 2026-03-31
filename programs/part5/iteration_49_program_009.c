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
        c[i] = i;  // Sequential values
        d[i] = rand() % 65535;
        e[i] = (long)rand() * rand();
    }
    
    // Destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        int cmp1 = (a[i] > b[i]) ? 1 : 0;
        // Array element vs constant comparison
        int cmp2 = (c[i] > 500) ? 2 : 0;
        // Store combined result
        gt_result[i] = cmp1 + cmp2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Different data types to test various integer comparisons
        int cmp1 = (a[i] >= b[i]) ? 3 : 0;
        // Unsigned short comparison
        int cmp2 = (d[i] >= 32768) ? 4 : 0;
        // Store combined result
        ge_result[i] = cmp1 + cmp2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison with swapped operands
        int cmp1 = (b[i] < a[i]) ? 5 : 0;
        // Long integer comparison
        int cmp2 = (e[i] < 1000000L) ? 6 : 0;
        // Array element vs constant
        int cmp3 = (c[i] < 200) ? 7 : 0;
        // Store combined result
        lt_result[i] = cmp1 + cmp2 + cmp3;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Multiple comparisons in one loop
        int cmp1 = (a[i] <= b[i]) ? 8 : 0;
        int cmp2 = (c[i] <= 800) ? 9 : 0;
        // Different data type
        int cmp3 = (d[i] <= 10000) ? 10 : 0;
        // Store combined result
        le_result[i] = cmp1 + cmp2 + cmp3;
    }
    
    // Compute checksum to ensure all loops have observable effects
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification: print first few results
    printf("First 5 results:\n");
    for (int i = 0; i < 5 && i < N; ++i) {
        printf("  [%d]: gt=%d, ge=%d, lt=%d, le=%d\n", 
               i, gt_result[i], ge_result[i], lt_result[i], le_result[i]);
    }
    
    return 0;
}
