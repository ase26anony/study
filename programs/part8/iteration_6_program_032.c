#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED char gt_results[N];  // Results for greater-than comparisons
    ALIGNED char le_results[N];  // Results for less-or-equal comparisons
    ALIGNED char lt_results[N];  // Results for less-than comparisons  
    ALIGNED char ge_results[N];  // Results for greater-or-equal comparisons
    
    // Initialize arrays with varying patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Values 0-99
        b[i] = (i * 7) % 100;      // Different pattern 0-99
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) - should trigger transformation for greater-than
    for (int i = 0; i < N; i++) {
        gt_results[i] = (a[i] > b[i]);  // Store as char to reduce memory usage
        sum += gt_results[i];  // Use result to prevent elimination
    }
    
    // Loop 2: LE_EXPR (<=) - should trigger transformation for less-or-equal
    for (int i = 0; i < N; i++) {
        le_results[i] = (a[i] <= b[i]);
        sum += le_results[i];
    }
    
    // Loop 3: LT_EXPR (<) - should trigger transformation for less-than
    for (int i = 0; i < N; i++) {
        lt_results[i] = (a[i] < b[i]);
        sum += lt_results[i];
    }
    
    // Loop 4: GE_EXPR (>=) - should trigger transformation for greater-or-equal
    for (int i = 0; i < N; i++) {
        ge_results[i] = (a[i] >= b[i]);
        sum += ge_results[i];
    }
    
    // Additional test: Mixed comparisons in same loop
    ALIGNED char mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Use both > and <= in conditional expressions
        if (a[i] > b[i]) {
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {
            mixed_results[i] = 2;
        }
        sum += mixed_results[i];
    }
    
    // Use results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    int verify_count = 0;
    for (int i = 0; i < 10; i++) {
        if (gt_results[i] == (a[i] > b[i])) verify_count++;
        if (le_results[i] == (a[i] <= b[i])) verify_count++;
        if (lt_results[i] == (a[i] < b[i])) verify_count++;
        if (ge_results[i] == (a[i] >= b[i])) verify_count++;
    }
    
    return (verify_count == 40) ? 0 : 1;
}
