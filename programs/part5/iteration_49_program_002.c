#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare source arrays with distinct data
    int a[N], b[N], c[N];
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize source arrays with reproducible pseudo-random data
    srand(42);
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = (i * 3) % 1000;  // Different pattern for variety
    }
    
    // Loop 1: Greater-than comparisons (GT_EXPR)
    for (int i = 0; i < N; ++i) {
        // Compare array elements with each other
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            gt_result[i] = 0;
        }
    }
    
    // Loop 2: Greater-than-or-equal comparisons (GE_EXPR)
    const int GE_LIMIT = 500;  // Loop-invariant constant
    for (int i = 0; i < N; ++i) {
        // Compare array element with constant
        if (a[i] >= GE_LIMIT) {
            ge_result[i] = a[i];  // Store original value if true
        } else {
            ge_result[i] = 0;     // Store 0 if false
        }
    }
    
    // Loop 3: Less-than comparisons (LT_EXPR)
    for (int i = 0; i < N; ++i) {
        // Compare array elements with each other
        if (b[i] < c[i]) {
            lt_result[i] = b[i] + c[i];  // Some computation
        } else {
            lt_result[i] = b[i] - c[i];  // Different computation
        }
    }
    
    // Loop 4: Less-than-or-equal comparisons (LE_EXPR)
    const int LE_LIMIT = 750;  // Loop-invariant constant
    for (int i = 0; i < N; ++i) {
        // Compare array element with constant
        if (c[i] <= LE_LIMIT) {
            le_result[i] = c[i] * 2;  // Store doubled value
        } else {
            le_result[i] = c[i] / 2;  // Store halved value
        }
    }
    
    // Additional loops with different integer types to increase coverage
    unsigned short us_a[N], us_b[N];
    unsigned short us_result[N];
    
    for (int i = 0; i < N; ++i) {
        us_a[i] = (unsigned short)(rand() % 65535);
        us_b[i] = (unsigned short)(rand() % 65535);
    }
    
    // Loop 5: Greater-than with unsigned short
    for (int i = 0; i < N; ++i) {
        us_result[i] = (us_a[i] > us_b[i]) ? us_a[i] : us_b[i];
    }
    
    // Loop 6: Less-than-or-equal with unsigned short
    const unsigned short US_LIMIT = 32768;
    for (int i = 0; i < N; ++i) {
        us_result[i] = (us_a[i] <= US_LIMIT) ? us_a[i] : 0;
    }
    
    // Compute checksum to ensure all computations are used
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += us_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
