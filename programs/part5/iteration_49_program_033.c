#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024

int main() {
    // Initialize source arrays with non-uniform data
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Seed for reproducibility
    srand(42);
    
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 3) % 100;          // Varying values 0-99
        b[i] = (i * 5) % 100;          // Different pattern
        c[i] = (i * 7) % 100;          // Another pattern
        d[i] = (unsigned short)((i * 11) % 256);  // Different type
        e[i] = (long)((i * 13) % 200); // Long type
    }
    
    // Destination arrays for each comparison type
    int gt_result[N];
    int ge_result[N];
    int lt_result[N];
    int le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        int cond1 = (a[i] > b[i]) ? 1 : 0;
        // Array element vs constant
        int cond2 = (c[i] > 50) ? 2 : 0;
        // Combine results with side effect
        gt_result[i] = cond1 + cond2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    // Using different data types and patterns
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison with unsigned short
        int cond1 = (d[i] >= (unsigned short)b[i]) ? 3 : 0;
        // Array-to-constant with different type
        int cond2 = (a[i] >= 25) ? 4 : 0;
        // Store combined result
        ge_result[i] = cond1 + cond2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Using long data type and mixing comparisons
    for (int i = 0; i < N; ++i) {
        // Long type comparison
        int cond1 = (e[i] < (long)c[i]) ? 5 : 0;
        // Array-to-constant with negative values
        int cond2 = (a[i] < 75) ? 6 : 0;
        // Store result
        lt_result[i] = cond1 + cond2;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    // Multiple comparisons in same loop
    for (int i = 0; i < N; ++i) {
        // Array-to-array
        int cond1 = (b[i] <= a[i]) ? 7 : 0;
        // Array-to-constant with different threshold
        int cond2 = (d[i] <= 128) ? 8 : 0;
        // Another array-to-array
        int cond3 = (c[i] <= e[i]) ? 9 : 0;
        // Store combined result
        le_result[i] = cond1 + cond2 + cond3;
    }
    
    // Additional loops with different patterns to increase coverage
    
    // Loop 5: Mixed GT/GE comparisons in same loop
    int mixed_result1[N];
    for (int i = 0; i < N; ++i) {
        int val1 = (a[i] > 10) ? 1 : 0;    // GT_EXPR
        int val2 = (b[i] >= 20) ? 2 : 0;   // GE_EXPR
        mixed_result1[i] = val1 + val2;
    }
    
    // Loop 6: Mixed LT/LE comparisons in same loop
    int mixed_result2[N];
    for (int i = 0; i < N; ++i) {
        int val1 = (c[i] < 80) ? 3 : 0;    // LT_EXPR
        int val2 = (d[i] <= 200) ? 4 : 0;  // LE_EXPR
        mixed_result2[i] = val1 + val2;
    }
    
    // Loop 7: Nested comparisons to test complex conditions
    int nested_result[N];
    for (int i = 0; i < N; ++i) {
        // Nested condition: (a > b) && (c <= d)
        int cond = (a[i] > b[i]) ? ((c[i] <= d[i]) ? 10 : 0) : 0;
        nested_result[i] = cond;
    }
    
    // Compute checksum to ensure all computations are used
    uint64_t checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += mixed_result1[i];
        checksum += mixed_result2[i];
        checksum += nested_result[i];
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    // Additional verification: print first few results
    printf("First 5 results from each array:\n");
    printf("GT: ");
    for (int i = 0; i < 5; ++i) printf("%d ", gt_result[i]);
    printf("\nGE: ");
    for (int i = 0; i < 5; ++i) printf("%d ", ge_result[i]);
    printf("\nLT: ");
    for (int i = 0; i < 5; ++i) printf("%d ", lt_result[i]);
    printf("\nLE: ");
    for (int i = 0; i < 5; ++i) printf("%d ", le_result[i]);
    printf("\n");
    
    return 0;
}
