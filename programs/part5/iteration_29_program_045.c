#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
int device_compute(int x, int y) {
    return x * y + (x % 7) - (y % 5);
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int result = 0;
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = i * 2 + (i % 13);
        b[i] = i * 3 - (i % 11);
    }
    
    // Use argc as runtime condition for offloading decision
    // This creates conditional execution paths
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // Complex nested OpenMP construct for GPU offloading with SIMD
        // This should trigger the SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(tofrom: result) \
            reduction(+:result) num_teams(4) thread_limit(128)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop body with device function call
            // and data-dependent computation
            int temp = device_compute(a[i], b[i]);
            result += temp * (i % 17 + 1);
            
            // Additional computation to create substantial GIMPLE
            if (temp > 100) {
                result -= (temp % 19);
            } else {
                result += (temp % 23);
            }
        }
    } else {
        // Host fallback version - still with OpenMP but no offloading
        #pragma omp parallel for simd reduction(+:result)
        for (int i = 0; i < N; i++) {
            int temp = a[i] * b[i] + (a[i] % 7) - (b[i] % 5);
            result += temp * (i % 17 + 1);
            
            if (temp > 100) {
                result -= (temp % 19);
            } else {
                result += (temp % 23);
            }
        }
    }
    
    // Print result to prevent optimization removal
    printf("Result: %d\n", result);
    
    // Additional checksum to ensure computation happened
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum ^= a[i] ^ b[i];
    }
    printf("Checksum: %d\n", checksum);
    
    free(a);
    free(b);
    
    return 0;
}
