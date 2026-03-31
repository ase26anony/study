#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare source arrays with different data patterns
    int a[N], b[N], c[N];
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize arrays with reproducible pseudo-random data
    srand(42);
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = i;  // Simple increasing pattern
    }
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            gt_result[i] = 0;
        }
        
        // Also include a comparison with constant
        if (a[i] > 500) {
            gt_result[i] += 2;  // Add to existing value
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (a[i] >= b[i]) {
            ge_result[i] = 1;
        } else {
            ge_result[i] = 0;
        }
        
        // Comparison with loop-invariant constant
        if (c[i] >= 250) {
            ge_result[i] += 2;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (a[i] < b[i]) {
            lt_result[i] = 1;
        } else {
            lt_result[i] = 0;
        }
        
        // Comparison with constant
        if (c[i] < 750) {
            lt_result[i] += 2;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (a[i] <= b[i]) {
            le_result[i] = 1;
        } else {
            le_result[i] = 0;
        }
        
        // Comparison with constant
        if (c[i] <= 500) {
            le_result[i] += 2;
        }
    }
    
    // Additional loops with different integer types to increase coverage
    unsigned short us_a[N], us_b[N];
    unsigned short us_result[N];
    
    for (int i = 0; i < N; i++) {
        us_a[i] = (unsigned short)(a[i] % 65535);
        us_b[i] = (unsigned short)(b[i] % 65535);
    }
    
    // Loop with unsigned short type and > comparison
    for (int i = 0; i < N; i++) {
        if (us_a[i] > us_b[i]) {
            us_result[i] = 1;
        } else {
            us_result[i] = 0;
        }
    }
    
    // Loop with long type and <= comparison
    long long la[N], lb[N];
    long long ll_result[N];
    
    for (int i = 0; i < N; i++) {
        la[i] = (long long)a[i] * 1000;
        lb[i] = (long long)b[i] * 1000;
    }
    
    for (int i = 0; i < N; i++) {
        if (la[i] <= lb[i]) {
            ll_result[i] = 1;
        } else {
            ll_result[i] = 0;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
        checksum += us_result[i] + ll_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification: count true comparisons
    int gt_count = 0, ge_count = 0, lt_count = 0, le_count = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) gt_count++;
        if (a[i] >= b[i]) ge_count++;
        if (a[i] < b[i]) lt_count++;
        if (a[i] <= b[i]) le_count++;
    }
    
    printf("GT: %d, GE: %d, LT: %d, LE: %d\n", 
           gt_count, ge_count, lt_count, le_count);
    
    return 0;
}
