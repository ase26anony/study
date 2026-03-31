#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int result_gt[N];
    ALIGNED int result_ge[N];
    ALIGNED int result_lt[N];
    ALIGNED int result_le[N];
    
    // Initialize with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) - should trigger transformation for greater-than
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];
        sum += result_gt[i];  // Prevent elimination
    }
    
    // Loop 2: GE_EXPR (>=) - should trigger transformation for greater-or-equal
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i];
        sum += result_ge[i];
    }
    
    // Loop 3: LT_EXPR (<) - should trigger transformation for less-than
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];
        sum += result_lt[i];
    }
    
    // Loop 4: LE_EXPR (<=) - should trigger transformation for less-or-equal
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i];
        sum += result_le[i];
    }
    
    // Also test with mixed comparisons in same loop
    ALIGNED int mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Mix of all four comparison types
        if (a[i] > b[i]) {
            mixed_results[i] = 1;
        } else if (a[i] >= b[i]) {
            mixed_results[i] = 2;
        } else if (a[i] < b[i]) {
            mixed_results[i] = 3;
        } else if (a[i] <= b[i]) {
            mixed_results[i] = 4;
        } else {
            mixed_results[i] = 0;
        }
        sum += mixed_results[i];
    }
    
    // Use results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Additional test with different integer types
    ALIGNED short short_a[N];
    ALIGNED short short_b[N];
    ALIGNED short short_result[N];
    
    for (int i = 0; i < N; i++) {
        short_a[i] = (i * 5) % 100;
        short_b[i] = (i * 11) % 100;
    }
    
    // Test with short type using <= operator
    for (int i = 0; i < N; i++) {
        short_result[i] = short_a[i] <= short_b[i];
        sum += short_result[i];
    }
    
    // Test with unsigned char type using >= operator
    ALIGNED unsigned char char_a[N];
    ALIGNED unsigned char char_b[N];
    ALIGNED unsigned char char_result[N];
    
    for (int i = 0; i < N; i++) {
        char_a[i] = (i * 13) % 256;
        char_b[i] = (i * 17) % 256;
    }
    
    for (int i = 0; i < N; i++) {
        char_result[i] = char_a[i] >= char_b[i];
        sum += char_result[i];
    }
    
    printf("Final checksum: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
