#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGN __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    int ALIGN a[N];
    int ALIGN b[N];
    int ALIGN results_gt[N];
    int ALIGN results_le[N];
    int ALIGN results_lt[N];
    int ALIGN results_ge[N];
    
    // Initialize arrays with distinct values
    for (int i = 0; i < N; i++) {
        a[i] = i * 2;
        b[i] = i * 3 - 100;
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) - should trigger vectorization
    for (int i = 0; i < N; i++) {
        results_gt[i] = a[i] > b[i];
        sum += results_gt[i];
    }
    
    // Loop 2: LE_EXPR (<=) - different comparison type
    for (int i = 0; i < N; i++) {
        results_le[i] = a[i] <= b[i];
        sum += results_le[i];
    }
    
    // Loop 3: LT_EXPR (<) - third comparison type
    for (int i = 0; i < N; i++) {
        results_lt[i] = a[i] < b[i];
        sum += results_lt[i];
    }
    
    // Loop 4: GE_EXPR (>=) - fourth comparison type
    for (int i = 0; i < N; i++) {
        results_ge[i] = a[i] >= b[i];
        sum += results_ge[i];
    }
    
    // Use results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Additional usage to ensure comparisons aren't optimized away
    int count_gt = 0, count_le = 0, count_lt = 0, count_ge = 0;
    for (int i = 0; i < N; i++) {
        if (results_gt[i]) count_gt++;
        if (results_le[i]) count_le++;
        if (results_lt[i]) count_lt++;
        if (results_ge[i]) count_ge++;
    }
    
    printf("GT: %d, LE: %d, LT: %d, GE: %d\n", 
           count_gt, count_le, count_lt, count_ge);
    
    return (sum > 0) ? 0 : 1;
}
