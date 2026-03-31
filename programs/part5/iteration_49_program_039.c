#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare source arrays with distinct data
    int a[N], b[N], c[N];
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize arrays with reproducible pseudo-random data
    srand(42);
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = (i * 3) % 1000;  // Different pattern for variety
    }
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix array-to-array and array-to-constant comparisons
    const int GT_CONST = 500;
    for (int i = 0; i < N; ++i) {
        // Array element comparison (a[i] > b[i])
        int cond1 = (a[i] > b[i]) ? 1 : 0;
        // Array element vs constant comparison (a[i] > CONST)
        int cond2 = (a[i] > GT_CONST) ? 2 : 0;
        // Combine results with bitwise OR to ensure both comparisons are used
        gt_result[i] = cond1 | cond2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    const int GE_CONST = 300;
    for (int i = 0; i < N; ++i) {
        // Array element comparison (b[i] >= c[i])
        int cond1 = (b[i] >= c[i]) ? 1 : 0;
        // Array element vs constant comparison (b[i] >= CONST)
        int cond2 = (b[i] >= GE_CONST) ? 2 : 0;
        // Use different combination to avoid identical patterns
        ge_result[i] = cond1 + cond2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    const int LT_CONST = 700;
    for (int i = 0; i < N; ++i) {
        // Array element comparison (c[i] < a[i])
        int cond1 = (c[i] < a[i]) ? 1 : 0;
        // Array element vs constant comparison (c[i] < CONST)
        int cond2 = (c[i] < LT_CONST) ? 2 : 0;
        // Different combination pattern
        lt_result[i] = cond1 * 3 + cond2;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    const int LE_CONST = 600;
    for (int i = 0; i < N; ++i) {
        // Array element comparison (a[i] <= b[i])
        int cond1 = (a[i] <= b[i]) ? 1 : 0;
        // Array element vs constant comparison (a[i] <= CONST)
        int cond2 = (a[i] <= LE_CONST) ? 2 : 0;
        // Different combination pattern
        le_result[i] = (cond1 << 1) | cond2;
    }
    
    // Compute checksum to prevent dead code elimination
    unsigned long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
