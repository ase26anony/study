#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
int device_func(int x) {
    return x * 2;
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    int *array_a = (int *)malloc(N * sizeof(int));
    int *array_b = (int *)malloc(N * sizeof(int));
    int result = 0;
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        array_a[i] = i % 100;
        array_b[i] = (i * 3) % 100;
    }
    
    // Use argc as runtime condition for offloading decision
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // Offloaded path - should trigger SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            map(to: array_a[0:N], array_b[0:N]) \
            map(tofrom: result) \
            reduction(+:result)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop body with device function call
            int temp = device_func(array_a[i]) + array_b[i];
            result += temp * (i % 10);  // Data-dependent computation
        }
        
        printf("Offloaded execution result: %d\n", result);
    } else {
        // Host fallback path
        for (int i = 0; i < N; i++) {
            int temp = (array_a[i] * 2) + array_b[i];
            result += temp * (i % 10);
        }
        
        printf("Host execution result: %d\n", result);
    }
    
    // Additional nested construct to increase complexity
    int partial_results[4] = {0, 0, 0, 0};
    
    #pragma omp target teams distribute parallel for simd \
        map(to: array_a[0:N]) \
        map(from: partial_results[0:4]) \
        num_teams(2) num_threads(128)
    for (int i = 0; i < N; i++) {
        int lane = i % 4;
        partial_results[lane] += array_a[i] * (i + 1);
    }
    
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += partial_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(array_a);
    free(array_b);
    
    return 0;
}
