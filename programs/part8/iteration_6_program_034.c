#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

// Function to prevent optimization
static void use_result(int *result) {
    volatile int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += result[i];
    }
    printf("Checksum: %d\n", sum);
}

int main(void) {
    // Aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int result1[N];  // For > comparisons
    ALIGNED int result2[N];  // For <= comparisons
    ALIGNED int result3[N];  // For < comparisons  
    ALIGNED int result4[N];  // For >= comparisons
    
    // Initialize with pattern that ensures mixed true/false results
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = (i % 3) * 100 + (i % 5) * 10;
    }
    
    // Loop 1: GT_EXPR (> operator) - uncovered case
    for (int i = 0; i < N; i++) {
        result1[i] = a[i] > b[i];
    }
    
    // Loop 2: LE_EXPR (<= operator) - uncovered case
    for (int i = 0; i < N; i++) {
        result2[i] = a[i] <= b[i];
    }
    
    // Loop 3: LT_EXPR (< operator) - uncovered case
    for (int i = 0; i < N; i++) {
        result3[i] = a[i] < b[i];
    }
    
    // Loop 4: GE_EXPR (>= operator) - uncovered case
    for (int i = 0; i < N; i++) {
        result4[i] = a[i] >= b[i];
    }
    
    // Use results to prevent dead code elimination
    use_result(result1);
    use_result(result2);
    use_result(result3);
    use_result(result4);
    
    // Also test with scalar comparison in conditional
    int count_gt = 0;
    int count_le = 0;
    
    // Mixed comparisons in same loop
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {      // GT_EXPR
            count_gt++;
        }
        if (a[i] <= b[i]) {     // LE_EXPR
            count_le++;
        }
    }
    
    printf("Count > : %d\n", count_gt);
    printf("Count <=: %d\n", count_le);
    
    // Additional test with different integer types
    ALIGNED short sa[N], sb[N];
    ALIGNED short sresult[N];
    
    for (int i = 0; i < N; i++) {
        sa[i] = i % 100;
        sb[i] = (i * 7) % 100;
    }
    
    // Short comparisons - might trigger different vectorization
    for (int i = 0; i < N; i++) {
        sresult[i] = sa[i] >= sb[i];  // GE_EXPR with short
    }
    
    volatile short ssum = 0;
    for (int i = 0; i < N; i++) {
        ssum += sresult[i];
    }
    
    return 0;
}
