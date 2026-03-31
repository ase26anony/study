#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define CONST_LIMIT 500

int main() {
    // Initialize source arrays with non-uniform data
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Seed for reproducibility
    srand(42);
    
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = i % 256;  // Simple pattern
        d[i] = (unsigned short)(rand() % 65536);
        e[i] = (long)(rand() % 2000) - 1000;  // Signed values
    }
    
    // Destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        int cond1 = (a[i] > b[i]) ? 1 : 0;
        // Array element vs constant comparison
        int cond2 = (c[i] > CONST_LIMIT) ? 2 : 0;
        // Combine results with bitwise OR to ensure both comparisons are used
        gt_result[i] = cond1 | cond2;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    // Use different data types to test various integer comparisons
    for (int i = 0; i < N; i++) {
        // Unsigned short comparison (different integer type)
        int cond1 = (d[i] >= (unsigned short)CONST_LIMIT) ? 1 : 0;
        // Long comparison (different integer type)
        int cond2 = (e[i] >= 0L) ? 2 : 0;
        // Combine with addition to ensure both are used
        ge_result[i] = cond1 + cond2;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Test with swapped operands in some cases
    for (int i = 0; i < N; i++) {
        // Standard array comparison
        int cond1 = (a[i] < b[i]) ? 1 : 0;
        // Comparison with constant on right side
        int cond2 = (c[i] < CONST_LIMIT) ? 2 : 0;
        // Comparison with constant on left side (tests operand swapping)
        int cond3 = (100 < d[i]) ? 4 : 0;
        lt_result[i] = cond1 | cond2 | cond3;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    // Mix of different comparison patterns
    for (int i = 0; i < N; i++) {
        // Array comparison
        int cond1 = (b[i] <= a[i]) ? 1 : 0;
        // Comparison with negative constant
        int cond2 = (e[i] <= -100L) ? 2 : 0;
        // Comparison with computed value
        int cond3 = (c[i] <= (i % 128)) ? 4 : 0;
        le_result[i] = cond1 + cond2 + cond3;
    }
    
    // Additional test: Nested comparisons to ensure complex patterns
    int complex_result[N];
    for (int i = 0; i < N; i++) {
        // Multiple comparisons in one expression
        int cond = (a[i] > b[i]) && (c[i] <= CONST_LIMIT);
        complex_result[i] = cond ? a[i] : b[i];
    }
    
    // Compute checksum to prevent dead code elimination
    uint64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += complex_result[i];
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    // Additional verification: count true conditions
    int gt_count = 0, ge_count = 0, lt_count = 0, le_count = 0;
    for (int i = 0; i < N; i++) {
        if (gt_result[i] & 1) gt_count++;
        if (ge_result[i] & 1) ge_count++;
        if (lt_result[i] & 1) lt_count++;
        if (le_result[i] & 1) le_count++;
    }
    
    printf("GT true: %d, GE true: %d, LT true: %d, LE true: %d\n",
           gt_count, ge_count, lt_count, le_count);
    
    return 0;
}
