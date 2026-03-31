#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

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
    
    // Loop 1: GT_EXPR (>)
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];
        sum += result_gt[i];
    }
    
    // Loop 2: GE_EXPR (>=)
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i];
        sum += result_ge[i];
    }
    
    // Loop 3: LT_EXPR (<)
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];
        sum += result_lt[i];
    }
    
    // Loop 4: LE_EXPR (<=)
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i];
        sum += result_le[i];
    }
    
    // Use results to prevent dead code elimination
    printf("Checksum: %d\n", sum);
    
    // Verify some results
    int verify = 0;
    for (int i = 0; i < N; i++) {
        verify += (result_gt[i] == (a[i] > b[i]));
        verify += (result_ge[i] == (a[i] >= b[i]));
        verify += (result_lt[i] == (a[i] < b[i]));
        verify += (result_le[i] == (a[i] <= b[i]));
    }
    
    if (verify == N * 4) {
        printf("All comparisons correct\n");
    } else {
        printf("Mismatch found\n");
    }
    
    return 0;
}
