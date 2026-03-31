#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Declare source arrays with distinct data
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Declare destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize source arrays with reproducible pseudo-random data
    srand(42);
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = (i * 3) % 1000;  // Different pattern
        d[i] = rand() % 65535;
        e[i] = (long)rand() * rand() % 1000000;
    }
    
    // Define loop-invariant constants for comparisons
    const int CONST_LIMIT = 500;
    const unsigned short USHORT_LIMIT = 30000;
    const long LONG_LIMIT = 300000L;
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Mix array-to-array and array-to-constant comparisons
        if (a[i] > b[i]) {
            gt_result[i] = 1;  // True case
        } else if (a[i] > CONST_LIMIT) {
            gt_result[i] = 2;  // Different value for constant comparison
        } else {
            gt_result[i] = 0;  // False case
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Use different data types and comparison patterns
        if (c[i] >= a[i]) {
            ge_result[i] = c[i] - a[i];  // Use difference as result
        } else if (d[i] >= USHORT_LIMIT) {
            ge_result[i] = 100;  // Constant result for unsigned short comparison
        } else {
            ge_result[i] = 0;
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Mix array-to-array comparisons with different operand order
        if (b[i] < a[i]) {
            lt_result[i] = a[i] - b[i];  // Positive difference
        } else if (e[i] < LONG_LIMIT) {
            lt_result[i] = 1;  // Simple constant for long comparison
        } else {
            lt_result[i] = 0;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Use multiple comparisons in the same loop
        if (a[i] <= b[i]) {
            le_result[i] = b[i] - a[i];  // Non-zero result
        } else if (c[i] <= CONST_LIMIT) {
            le_result[i] = 50;  // Different constant
        } else if (e[i] <= 2 * LONG_LIMIT) {
            le_result[i] = 25;  // Another constant with expression
        } else {
            le_result[i] = 0;
        }
    }
    
    // Additional loop with mixed comparison types to ensure all paths are exercised
    int mixed_result[N];
    for (int i = 0; i < N; ++i) {
        // Chain of different comparison operators
        if (a[i] > 700) {
            mixed_result[i] = 100;
        } else if (a[i] >= 400) {
            mixed_result[i] = 50;
        } else if (a[i] < 200) {
            mixed_result[i] = 10;
        } else if (a[i] <= 300) {
            mixed_result[i] = 5;
        } else {
            mixed_result[i] = 1;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += mixed_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
