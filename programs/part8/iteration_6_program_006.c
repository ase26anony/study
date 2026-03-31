#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    int ALIGNED a[N], b[N];
    int ALIGNED gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize arrays with distinct patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Varying values 0-99
        b[i] = (i * 7) % 100;      // Different pattern 0-99
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) transformation
    for (int i = 0; i < N; i++) {
        gt_result[i] = (a[i] > b[i]);  // Should trigger GT_EXPR path
        sum += gt_result[i];           // Prevent elimination
    }
    
    // Loop 2: GE_EXPR (>=) transformation  
    for (int i = 0; i < N; i++) {
        ge_result[i] = (a[i] >= b[i]); // Should trigger GE_EXPR path
        sum += ge_result[i];
    }
    
    // Loop 3: LT_EXPR (<) transformation
    for (int i = 0; i < N; i++) {
        lt_result[i] = (a[i] < b[i]);  // Should trigger LT_EXPR path
        sum += lt_result[i];
    }
    
    // Loop 4: LE_EXPR (<=) transformation
    for (int i = 0; i < N; i++) {
        le_result[i] = (a[i] <= b[i]); // Should trigger LE_EXPR path
        sum += le_result[i];
    }
    
    // Also test with mixed comparisons in same loop
    int mixed_sum = 0;
    for (int i = 0; i < N; i++) {
        // Mix different comparison types
        if (a[i] > b[i]) mixed_sum += 1;    // GT_EXPR
        if (a[i] >= b[i]) mixed_sum += 2;   // GE_EXPR  
        if (a[i] < b[i]) mixed_sum += 3;    // LT_EXPR
        if (a[i] <= b[i]) mixed_sum += 4;   // LE_EXPR
    }
    
    // Use results to prevent dead code elimination
    printf("Sum of comparisons: %d\n", sum);
    printf("Mixed sum: %d\n", mixed_sum);
    
    // Additional test with unsigned types (may trigger different paths)
    unsigned int ALIGNED ua[N], ub[N];
    unsigned int ALIGNED u_result[N];
    
    for (int i = 0; i < N; i++) {
        ua[i] = (i * 5) % 256;
        ub[i] = (i * 11) % 256;
    }
    
    unsigned int usum = 0;
    for (int i = 0; i < N; i++) {
        u_result[i] = (ua[i] <= ub[i]);  // LE_EXPR with unsigned
        usum += u_result[i];
    }
    
    printf("Unsigned sum: %u\n", usum);
    
    return (sum > 0) ? 0 : 1;  // Return value depends on results
}
