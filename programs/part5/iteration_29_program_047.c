#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
int device_compute(int x, int y) {
    return (x * y) % 100;
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int result = 0;
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    // Use argc as runtime condition to decide execution path
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // This should trigger the SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(tofrom: result) \
            reduction(+:result)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop body with device function call
            int temp = device_compute(a[i], b[i]);
            result += temp + (i % 10);
        }
        
        printf("Offloaded execution result: %d\n", result);
    } else {
        // Host fallback - similar computation without offloading
        #pragma omp parallel for simd reduction(+:result)
        for (int i = 0; i < N; i++) {
            int temp = (a[i] * b[i]) % 100;
            result += temp + (i % 10);
        }
        
        printf("Host execution result: %d\n", result);
    }
    
    // Additional computation to ensure the SIMT transformation is complex enough
    int checksum = 0;
    volatile int volatile_bound = N; // Prevent constant propagation
    
    // Nested OpenMP constructs for complex GIMPLE structure
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N]) map(from: checksum) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < volatile_bound; i++) {
        #pragma omp simd
        for (int j = 0; j < 4; j++) {
            checksum += a[i] * (j + 1);
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(a);
    free(b);
    
    return 0;
}
