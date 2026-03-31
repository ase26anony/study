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
        c[i] = rand() % 1000;
    }
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            gt_result[i] = 0;
        }
        
        // Array-to-constant comparison
        if (a[i] > 500) {
            gt_result[i] += 2;
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (a[i] >= b[i]) {
            ge_result[i] = 1;
        } else {
            ge_result[i] = 0;
        }
        
        // Array-to-constant comparison
        if (a[i] >= 250) {
            ge_result[i] += 2;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (a[i] < b[i]) {
            lt_result[i] = 1;
        } else {
            lt_result[i] = 0;
        }
        
        // Array-to-constant comparison
        if (a[i] < 750) {
            lt_result[i] += 2;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        if (a[i] <= b[i]) {
            le_result[i] = 1;
        } else {
            le_result[i] = 0;
        }
        
        // Array-to-constant comparison
        if (a[i] <= 600) {
            le_result[i] += 2;
        }
    }
    
    // Additional loops with different integer types
    unsigned short us_a[N], us_b[N];
    unsigned short us_result[N];
    
    for (int i = 0; i < N; ++i) {
        us_a[i] = (unsigned short)(rand() % 65535);
        us_b[i] = (unsigned short)(rand() % 65535);
    }
    
    // Loop 5: Greater-than with unsigned short
    for (int i = 0; i < N; ++i) {
        if (us_a[i] > us_b[i]) {
            us_result[i] = 1;
        } else {
            us_result[i] = 0;
        }
    }
    
    // Loop 6: Less-than-or-equal with unsigned short
    for (int i = 0; i < N; ++i) {
        if (us_a[i] <= 32768) {
            us_result[i] += 2;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i] + us_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
