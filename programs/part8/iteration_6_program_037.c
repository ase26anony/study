#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED char result_gt[N];  // For > comparisons
    ALIGNED char result_le[N];  // For <= comparisons
    ALIGNED char result_lt[N];  // For < comparisons  
    ALIGNED char result_ge[N];  // For >= comparisons
    
    // Initialize with pattern to ensure mixed comparison results
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = (i % 3 == 0) ? i + 1 : 
               (i % 3 == 1) ? i - 1 : i;
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) comparisons
    for (int i = 0; i < N; i++) {
        result_gt[i] = (a[i] > b[i]);
    }
    
    // Loop 2: LE_EXPR (<=) comparisons  
    for (int i = 0; i < N; i++) {
        result_le[i] = (a[i] <= b[i]);
    }
    
    // Loop 3: LT_EXPR (<) comparisons
    for (int i = 0; i < N; i++) {
        result_lt[i] = (a[i] < b[i]);
    }
    
    // Loop 4: GE_EXPR (>=) comparisons
    for (int i = 0; i < N; i++) {
        result_ge[i] = (a[i] >= b[i]);
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        sum += result_gt[i] + result_le[i] + result_lt[i] + result_ge[i];
    }
    
    // Print checksum to ensure all comparisons are executed
    printf("Checksum: %d\n", sum);
    
    // Additional test with mixed comparisons in same loop
    ALIGNED char result_mixed[N];
    for (int i = 0; i < N; i++) {
        // Mix > and <= in conditional expressions
        if (a[i] > b[i]) {
            result_mixed[i] = 1;
        } else if (a[i] <= b[i]) {
            result_mixed[i] = 2;
        }
    }
    
    // Another test with while loop containing comparisons
    int j = 0;
    int acc = 0;
    while (j < N) {
        if (a[j] < b[j]) acc += 1;
        if (a[j] >= b[j]) acc += 2;
        j++;
    }
    printf("Accumulator: %d\n", acc);
    
    return sum > 0 ? 0 : 1;
}
