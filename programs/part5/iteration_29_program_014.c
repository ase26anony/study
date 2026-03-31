#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
int device_compute(int x) {
    return x * 2 + 1;
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int result = 0;
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = i % 100;
        b[i] = (i * 3) % 100;
    }
    
    // Use argc as runtime condition to decide execution path
    // This encourages conditional label generation
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // Complex nested OpenMP construct for SIMT transformation
        // Non-constant trip count via function parameter (argc)
        int loop_bound = N;
        if (argc > 2) loop_bound = atoi(argv[2]) % N;
        
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:loop_bound], b[0:loop_bound]) \
            map(tofrom: result) reduction(+:result) \
            num_teams(4) thread_limit(128)
        for (int i = 0; i < loop_bound; i++) {
            // Non-trivial loop body with device function call
            int temp = device_compute(a[i]);
            result += temp * b[i] - a[i];
            
            // Additional computation to create substantial GIMPLE
            if (i % 2 == 0) {
                result += (a[i] << 2);
            } else {
                result -= (b[i] >> 1);
            }
        }
        
        printf("Offload execution: result = %d\n", result);
    } else {
        // Host fallback path - still with OpenMP but no offloading
        #pragma omp parallel for simd reduction(+:result)
        for (int i = 0; i < N; i++) {
            result += a[i] * b[i];
        }
        
        printf("Host execution: result = %d\n", result);
    }
    
    // Additional test with nested parallel regions
    int verify = 0;
    volatile int volatile_bound = N; // Prevent optimization
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: verify) \
        num_teams(2) num_threads(64)
    for (int i = 0; i < volatile_bound; i++) {
        // Complex conditional computation
        int val = a[i];
        for (int j = 0; j < 3; j++) {
            val = (val * 17 + 3) % 256;
        }
        verify ^= val + b[i];
    }
    
    printf("Verification checksum: %d\n", verify);
    
    free(a);
    free(b);
    
    return 0;
}
