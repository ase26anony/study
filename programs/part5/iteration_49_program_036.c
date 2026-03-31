#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define CONST_LIMIT 500

int main() {
    // Initialize arrays with distinct data
    int a[N], b[N], c[N];
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Seed RNG for reproducibility
    srand(42);
    
    // Initialize source arrays with non-uniform data
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;           // 0-999
        b[i] = (i * 3) % 1000;          // Pattern based on index
        c[i] = 800 - (i % 400);         // Another pattern
    }
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            gt_result[i] = 0;
        }
        
        // Additional comparison with constant to test different RHS
        if (a[i] > CONST_LIMIT) {
            gt_result[i] += 10;  // Modify result based on constant comparison
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    // Using different arrays to avoid dependencies
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (b[i] >= c[i]) {
            ge_result[i] = 2;
        } else {
            ge_result[i] = -1;
        }
        
        // Comparison with loop-invariant constant
        if (b[i] >= CONST_LIMIT) {
            ge_result[i] *= 3;  // Transform result
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Using unsigned types to ensure proper integer comparisons
    unsigned short us_a[N], us_b[N];
    for (int i = 0; i < N; i++) {
        us_a[i] = a[i] % 65535;
        us_b[i] = b[i] % 65535;
    }
    
    for (int i = 0; i < N; i++) {
        // Array element comparison with unsigned types
        if (us_a[i] < us_b[i]) {
            lt_result[i] = us_a[i];
        } else {
            lt_result[i] = us_b[i];
        }
        
        // Comparison with constant
        if (us_a[i] < CONST_LIMIT) {
            lt_result[i] += 1000;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    // Using long type for wider integer operations
    long la[N], lb[N];
    for (int i = 0; i < N; i++) {
        la[i] = a[i] * 100L;
        lb[i] = b[i] * 100L + 50L;
    }
    
    for (int i = 0; i < N; i++) {
        // Array element comparison with long type
        if (la[i] <= lb[i]) {
            le_result[i] = (int)(la[i] / 100);
        } else {
            le_result[i] = (int)(lb[i] / 100);
        }
        
        // Comparison with constant (scaled)
        if (la[i] <= (CONST_LIMIT * 100L)) {
            le_result[i] += 5000;
        }
    }
    
    // Additional test: Mixed comparisons in same loop
    // This might trigger multiple transformations
    int mixed_result[N];
    for (int i = 0; i < N; i++) {
        int temp = 0;
        if (a[i] > b[i]) temp += 1;      // GT_EXPR
        if (a[i] >= c[i]) temp += 2;     // GE_EXPR  
        if (b[i] < c[i]) temp += 4;      // LT_EXPR
        if (b[i] <= a[i]) temp += 8;     // LE_EXPR
        mixed_result[i] = temp;
    }
    
    // Compute checksum to ensure all computations are used
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += mixed_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification print
    printf("Sample results - first 5 elements:\n");
    for (int i = 0; i < 5 && i < N; i++) {
        printf("  [%d]: gt=%d, ge=%d, lt=%d, le=%d, mixed=%d\n",
               i, gt_result[i], ge_result[i], lt_result[i], le_result[i], mixed_result[i]);
    }
    
    return 0;
}
