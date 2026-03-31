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
        b[i] = rand() % 1000;
        c[i] = i % 256;  // Pattern for constant comparison
        d[i] = rand() % 65535;
        e[i] = (long)rand() * rand();
    }
    
    // Constants for comparisons
    const int CONST_LIMIT = 500;
    const unsigned short USHORT_LIMIT = 30000;
    const long LONG_LIMIT = 1000000L;
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison
        gt_result[i] = (a[i] > b[i]) ? 1 : 0;
        
        // Array-to-constant comparison
        if (d[i] > USHORT_LIMIT) {
            gt_result[i] += 2;  // Add different value for constant comparison
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    // Different data types and comparison patterns
    for (int i = 0; i < N; ++i) {
        // Array-to-array comparison with different types
        ge_result[i] = (a[i] >= c[i]) ? 3 : 1;
        
        // Array-to-constant comparison
        if (e[i] >= LONG_LIMIT) {
            ge_result[i] *= 2;  // Different operation for side effect
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Using swapped operands to trigger std::swap in the uncovered code
    for (int i = 0; i < N; ++i) {
        // Standard array-to-array comparison
        lt_result[i] = (b[i] < a[i]) ? 5 : 2;
        
        // Array-to-constant comparison
        if (c[i] < CONST_LIMIT) {
            lt_result[i] += c[i];  // Use array value in result
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    // More complex pattern with multiple comparisons
    for (int i = 0; i < N; ++i) {
        // Multiple comparisons in one loop iteration
        int temp = 0;
        if (a[i] <= b[i]) {
            temp += 1;
        }
        if (d[i] <= USHORT_LIMIT) {
            temp += 2;
        }
        if (e[i] <= LONG_LIMIT) {
            temp += 4;
        }
        le_result[i] = temp;
    }
    
    // Additional loop with mixed comparisons to ensure all paths are exercised
    int mixed_result[N];
    for (int i = 0; i < N; ++i) {
        // Chain of different comparisons
        mixed_result[i] = 0;
        mixed_result[i] |= (a[i] > CONST_LIMIT) ? 0x1 : 0;
        mixed_result[i] |= (b[i] >= a[i]) ? 0x2 : 0;
        mixed_result[i] |= (c[i] < 128) ? 0x4 : 0;
        mixed_result[i] |= (d[i] <= USHORT_LIMIT) ? 0x8 : 0;
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
    
    // Additional verification to ensure all loops executed
    printf("Sample results:\n");
    printf("gt_result[0]=%d, ge_result[100]=%d, lt_result[200]=%d, le_result[300]=%d\n",
           gt_result[0], ge_result[100], lt_result[200], le_result[300]);
    
    return 0;
}
