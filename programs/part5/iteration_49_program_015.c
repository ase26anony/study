#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Initialize source arrays with distinct patterns
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Initialize with reproducible pseudo-random values
    srand(42);
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = i % 100;
        d[i] = (unsigned short)(rand() % 65535);
        e[i] = (long)(rand() % 10000);
    }
    
    // Destination arrays for each comparison type
    int gt_result[N];
    int ge_result[N];
    int lt_result[N];
    int le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    const int GT_THRESHOLD = 500;
    for (int i = 0; i < N; i++) {
        // Array element comparison
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            gt_result[i] = 0;
        }
        
        // Additional array-to-constant comparison
        if (c[i] > GT_THRESHOLD) {
            gt_result[i] += 2;
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    // Using different data types to test various integer comparisons
    const unsigned short GE_THRESHOLD = 32768;
    for (int i = 0; i < N; i++) {
        // Array-to-array comparison with different types
        if (d[i] >= (unsigned short)(i % 256)) {
            ge_result[i] = d[i] & 0xFF;
        } else {
            ge_result[i] = 0;
        }
        
        // Array-to-constant comparison
        if (d[i] >= GE_THRESHOLD) {
            ge_result[i] |= 0x100;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Using long data type for wider integer comparisons
    const long LT_LIMIT = 5000L;
    for (int i = 0; i < N; i++) {
        // Array-to-array comparison
        if (e[i] < (long)(a[i] * 10L)) {
            lt_result[i] = (int)(e[i] % 100);
        } else {
            lt_result[i] = -1;
        }
        
        // Array-to-constant comparison
        if (e[i] < LT_LIMIT) {
            lt_result[i] += 1000;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    // Mix of signed and unsigned comparisons
    const int LE_LIMIT = 800;
    for (int i = 0; i < N; i++) {
        // Array-to-array comparison
        if (a[i] <= b[i]) {
            le_result[i] = a[i] + b[i];
        } else {
            le_result[i] = a[i] - b[i];
        }
        
        // Array-to-constant comparison
        if (c[i] <= LE_LIMIT) {
            le_result[i] *= 2;
        }
    }
    
    // Additional loops with pure comparisons (no mixing) to ensure
    // each operator gets its own vectorization attempt
    
    // Pure GT loop
    int pure_gt[N];
    for (int i = 0; i < N; i++) {
        pure_gt[i] = (a[i] > b[i]) ? 1 : 0;
    }
    
    // Pure GE loop
    int pure_ge[N];
    for (int i = 0; i < N; i++) {
        pure_ge[i] = (d[i] >= GE_THRESHOLD) ? d[i] : 0;
    }
    
    // Pure LT loop
    int pure_lt[N];
    for (int i = 0; i < N; i++) {
        pure_lt[i] = (e[i] < LT_LIMIT) ? 1 : 0;
    }
    
    // Pure LE loop
    int pure_le[N];
    for (int i = 0; i < N; i++) {
        pure_le[i] = (c[i] <= LE_LIMIT) ? c[i] : 0;
    }
    
    // Compute checksum to ensure all computations are used
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += pure_gt[i];
        checksum += pure_ge[i];
        checksum += pure_lt[i];
        checksum += pure_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
