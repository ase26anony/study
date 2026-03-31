#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
int device_compute(int x, int y) {
    return (x * y) / (x + y + 1);
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int result = 0;
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 7 + 1;
        b[i] = (i * 5) % 11 + 1;
    }
    
    // Use argc as runtime condition for offloading decision
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // Complex nested OpenMP construct with SIMD clause
        // This should trigger the SIMT transformation for GPU offloading
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(from: result) reduction(+:result)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop body with device function call
            // and data-dependent computation
            int temp = device_compute(a[i], b[i]);
            result += temp * (i % 16 + 1);  // Varying weights
        }
        
        // Additional computation to ensure the transformation is needed
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: a[0:N]) map(to: b[0:N])
        for (int i = 0; i < N - 1; i++) {
            // Cross-iteration dependency pattern
            a[i] = (a[i] + b[i] + a[i + 1]) % 1000;
        }
    } else {
        // Host fallback version - similar computation without offloading
        #pragma omp parallel for simd reduction(+:result)
        for (int i = 0; i < N; i++) {
            int temp = (a[i] * b[i]) / (a[i] + b[i] + 1);
            result += temp * (i % 16 + 1);
        }
        
        #pragma omp parallel for simd
        for (int i = 0; i < N - 1; i++) {
            a[i] = (a[i] + b[i] + a[i + 1]) % 1000;
        }
    }
    
    // Compute checksum to prevent optimization removal
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum = (checksum + a[i]) % 1000000;
    }
    
    printf("Result: %d\n", result);
    printf("Checksum: %d\n", checksum);
    printf("Offload used: %s\n", use_offload ? "yes" : "no");
    
    free(a);
    free(b);
    
    return 0;
}
