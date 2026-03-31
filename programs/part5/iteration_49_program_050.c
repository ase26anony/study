#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024

int main() {
    // Seed for reproducible results
    srand(42);
    
    // Source arrays with distinct data
    int a[N], b[N], c[N];
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = (i * 3) % 1000;  // Different pattern
    }
    
    // Destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) comparisons
    // Mix of array-to-array and array-to-constant comparisons
    for (int i = 0; i < N; ++i) {
        // Array element comparison
        if (a[i] > b[i]) {
            gt_result[i] = 1;
        } else {
            gt_result[i] = 0;
        }
        
        // Also compare with constant
        if (a[i] > 500) {
            gt_result[i] += 10;  // Add to distinguish from pure 0/1
        }
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) comparisons
    for (int i = 0; i < N; ++i) {
        // Different arrays to avoid dependencies
        if (b[i] >= c[i]) {
            ge_result[i] = 2;
        } else {
            ge_result[i] = -1;
        }
        
        // Compare with different constant
        if (b[i] >= 250) {
            ge_result[i] *= 3;  // Different operation
        }
    }
    
    // Loop 3: Less-than (LT_EXPR) comparisons
    // Using unsigned types to ensure integer comparisons
    unsigned short us_a[N], us_b[N];
    for (int i = 0; i < N; ++i) {
        us_a[i] = (unsigned short)(a[i] % 65535);
        us_b[i] = (unsigned short)(b[i] % 65535);
    }
    
    for (int i = 0; i < N; ++i) {
        // Array element comparison (should trigger std::swap in uncovered code)
        if (us_a[i] < us_b[i]) {
            lt_result[i] = 100;
        } else {
            lt_result[i] = 200;
        }
        
        // Compare with constant
        if (us_a[i] < 30000) {
            lt_result[i] += 50;
        }
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) comparisons
    // Using long type for variety
    long la[N], lb[N];
    for (int i = 0; i < N; ++i) {
        la[i] = (long)a[i] * 1000L;
        lb[i] = (long)b[i] * 500L;
    }
    
    for (int i = 0; i < N; ++i) {
        // Array element comparison (should trigger std::swap in uncovered code)
        if (la[i] <= lb[i]) {
            le_result[i] = 1000;
        } else {
            le_result[i] = 2000;
        }
        
        // Compare with constant
        if (la[i] <= 500000L) {
            le_result[i] += 500;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification: count true comparisons
    int gt_count = 0, ge_count = 0, lt_count = 0, le_count = 0;
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) gt_count++;
        if (b[i] >= c[i]) ge_count++;
        if (us_a[i] < us_b[i]) lt_count++;
        if (la[i] <= lb[i]) le_count++;
    }
    
    printf("GT true: %d, GE true: %d, LT true: %d, LE true: %d\n",
           gt_count, ge_count, lt_count, le_count);
    
    return 0;
}
