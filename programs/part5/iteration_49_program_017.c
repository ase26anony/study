#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024

int main() {
    // Seed for reproducible results
    srand(42);
    
    // Source arrays with different data patterns
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize source arrays with varied data
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = (i * 3) % 1000;  // Different pattern from a
        c[i] = 500 - (i % 100); // Values around 500
        d[i] = (unsigned short)(rand() % 65535);
        e[i] = (long)(rand() % 10000) * (i % 2 == 0 ? 1 : -1); // Mix positive and negative
    }
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        int cond1 = a[i] > b[i];
        // Array element vs constant comparison
        int cond2 = c[i] > 250;
        // Combine conditions with bitwise operations
        gt_result[i] = (cond1 && cond2) ? 1 : 0;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    // Use different data types and comparison patterns
    for (int i = 0; i < N; ++i) {
        // Comparison with unsigned short
        int cond1 = d[i] >= 32768;
        // Comparison with negative values
        int cond2 = e[i] >= -1000;
        // Store result based on combined condition
        ge_result[i] = (cond1 || cond2) ? 2 : 0;
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // This should trigger std::swap(cond_expr0, cond_expr1)
    for (int i = 0; i < N; ++i) {
        // Multiple less-than comparisons
        int cond1 = a[i] < b[i];
        int cond2 = c[i] < 750;
        // Use bitwise AND to combine conditions
        lt_result[i] = (cond1 & cond2) ? 3 : 0;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    // Mix different integer types and comparison directions
    for (int i = 0; i < N; ++i) {
        // Comparison with different array
        int cond1 = b[i] <= c[i];
        // Comparison with constant
        int cond2 = e[i] <= 5000L;
        // Use bitwise OR to combine conditions
        le_result[i] = (cond1 | cond2) ? 4 : 0;
    }
    
    // Additional loop with more complex pattern for LT_EXPR
    // to ensure coverage of the swap logic
    int lt_extra[N];
    for (int i = 0; i < N; ++i) {
        // This pattern might better trigger the swap transformation
        lt_extra[i] = (a[i] < 100 && b[i] < 200) ? 5 : 0;
    }
    
    // Additional loop for LE_EXPR with different pattern
    int le_extra[N];
    for (int i = 0; i < N; ++i) {
        le_extra[i] = (c[i] <= 400 || d[i] <= 10000) ? 6 : 0;
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
        checksum += lt_extra[i] + le_extra[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification to ensure all loops execute
    int gt_count = 0, ge_count = 0, lt_count = 0, le_count = 0;
    for (int i = 0; i < N; ++i) {
        if (gt_result[i]) gt_count++;
        if (ge_result[i]) ge_count++;
        if (lt_result[i]) lt_count++;
        if (le_result[i]) le_count++;
    }
    
    printf("GT true: %d, GE true: %d, LT true: %d, LE true: %d\n",
           gt_count, ge_count, lt_count, le_count);
    
    return 0;
}
