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
        a[i] = (i * 3) % 97;
        b[i] = (i * 7) % 113;
    }
    
    // Runtime condition to choose execution path
    // (simulating conditional offloading)
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // Complex nested OpenMP construct for SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(tofrom: result) \
            reduction(+:result) num_teams(4) thread_limit(128)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop body with device function call
            // and data-dependent computation
            int temp = device_compute(a[i], b[i]);
            result += temp * (i % 16 + 1);  // Varying weights
        }
    } else {
        // Host fallback path
        #pragma omp parallel for simd reduction(+:result)
        for (int i = 0; i < N; i++) {
            int temp = (a[i] * b[i]) / (a[i] + b[i] + 1);
            result += temp * (i % 16 + 1);
        }
    }
    
    // Additional complexity: Nested parallel region with SIMD
    int partial_results[4] = {0};
    volatile int volatile_bound = N/4;  // Prevent optimization
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N]) map(from: partial_results[0:4]) \
        num_teams(2)
    for (int t = 0; t < 4; t++) {
        int start = t * volatile_bound;
        int end = (t + 1) * volatile_bound;
        int local_sum = 0;
        
        // Inner loop with SIMD - creates complex gomp_for structure
        #pragma omp simd reduction(+:local_sum)
        for (int i = start; i < end && i < N; i++) {
            local_sum += a[i] * (i % 8 + 1);
        }
        partial_results[t] = local_sum;
    }
    
    // Final computation and output
    int final_sum = result;
    for (int i = 0; i < 4; i++) {
        final_sum += partial_results[i];
    }
    
    printf("Result: %d\n", final_sum);
    printf("Checksum: %ld\n", (long)final_sum * 31);
    
    free(a);
    free(b);
    
    return 0;
}
