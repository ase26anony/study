#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Function to prevent optimization
static void use_result(int *result) {
    volatile int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += result[i];
    }
    printf("Checksum: %d\n", sum);
}

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int result1[N];  // For > comparisons
    ALIGNED int result2[N];  // For <= comparisons
    ALIGNED int result3[N];  // For < comparisons
    ALIGNED int result4[N];  // For >= comparisons
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    // Loop 1: GT_EXPR (>)
    // This should trigger the transformation for greater-than comparisons
    for (int i = 0; i < N; i++) {
        result1[i] = (a[i] > b[i]) ? 1 : 0;
    }
    
    // Loop 2: LE_EXPR (<=)
    // This should trigger the transformation for less-than-or-equal comparisons
    for (int i = 0; i < N; i++) {
        result2[i] = (a[i] <= b[i]) ? 1 : 0;
    }
    
    // Loop 3: LT_EXPR (<)
    // This should trigger the transformation for less-than comparisons
    for (int i = 0; i < N; i++) {
        result3[i] = (a[i] < b[i]) ? 1 : 0;
    }
    
    // Loop 4: GE_EXPR (>=)
    // This should trigger the transformation for greater-than-or-equal comparisons
    for (int i = 0; i < N; i++) {
        result4[i] = (a[i] >= b[i]) ? 1 : 0;
    }
    
    // Use results to prevent dead code elimination
    use_result(result1);
    use_result(result2);
    use_result(result3);
    use_result(result4);
    
    // Additional test with mixed comparisons in same loop
    ALIGNED int mixed_result[N];
    for (int i = 0; i < N; i++) {
        // Mix of different comparison types
        if (a[i] > b[i]) {
            mixed_result[i] = 1;
        } else if (a[i] <= b[i]) {
            mixed_result[i] = 2;
        } else if (a[i] < b[i]) {
            mixed_result[i] = 3;
        } else if (a[i] >= b[i]) {
            mixed_result[i] = 4;
        } else {
            mixed_result[i] = 0;
        }
    }
    use_result(mixed_result);
    
    return 0;
}
