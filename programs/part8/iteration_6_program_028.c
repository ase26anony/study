#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGN __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    int ALIGN a[N];
    int ALIGN b[N];
    int ALIGN results_gt[N];
    int ALIGN results_ge[N];
    int ALIGN results_lt[N];
    int ALIGN results_le[N];
    
    // Initialize arrays with distinct values
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Varying values 0-99
        b[i] = (i * 7) % 100;      // Different pattern 0-99
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>)
    // This should trigger the GT_EXPR transformation path
    for (int i = 0; i < N; i++) {
        results_gt[i] = (a[i] > b[i]) ? 1 : 0;
        sum += results_gt[i];  // Prevent elimination
    }
    
    // Loop 2: GE_EXPR (>=)
    // This should trigger the GE_EXPR transformation path
    for (int i = 0; i < N; i++) {
        results_ge[i] = (a[i] >= b[i]) ? 1 : 0;
        sum += results_ge[i];  // Prevent elimination
    }
    
    // Loop 3: LT_EXPR (<)
    // This should trigger the LT_EXPR transformation path
    for (int i = 0; i < N; i++) {
        results_lt[i] = (a[i] < b[i]) ? 1 : 0;
        sum += results_lt[i];  // Prevent elimination
    }
    
    // Loop 4: LE_EXPR (<=)
    // This should trigger the LE_EXPR transformation path
    for (int i = 0; i < N; i++) {
        results_le[i] = (a[i] <= b[i]) ? 1 : 0;
        sum += results_le[i];  // Prevent elimination
    }
    
    // Additional test: Mixed comparisons in same loop
    // This tests multiple comparison types together
    int mixed_results[N];
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {
            mixed_results[i] = 2;
        } else if (a[i] < b[i]) {
            mixed_results[i] = 3;
        } else if (a[i] >= b[i]) {
            mixed_results[i] = 4;
        }
        sum += mixed_results[i];
    }
    
    // Use results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    int verify_sum = 0;
    for (int i = 0; i < 10; i++) {
        verify_sum += results_gt[i] + results_ge[i] + 
                     results_lt[i] + results_le[i] + 
                     mixed_results[i];
    }
    printf("Verification sum (first 10 elements): %d\n", verify_sum);
    
    return (sum > 0) ? 0 : 1;
}
