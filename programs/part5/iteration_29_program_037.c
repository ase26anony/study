#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000
#define THRESHOLD 500

#pragma omp declare target
int device_compute(int x, int y) {
    return (x * y) % 37;
}
#pragma omp end declare target

void host_version(int *a, int *b, int *result) {
    int sum = 0;
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < N; i++) {
        int val = device_compute(a[i], b[i]);
        sum += val;
        a[i] = val;
    }
    *result = sum;
}

int main(int argc, char *argv[]) {
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int result = 0;
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 97;
        b[i] = (i * 7) % 113;
    }
    
    // Runtime condition for offloading decision
    int use_offload = 0;
    if (argc > 1) {
        use_offload = atoi(argv[1]);
    } else {
        // Check environment variable as alternative condition
        char *env = getenv("USE_OFFLOAD");
        if (env != NULL) {
            use_offload = atoi(env);
        }
    }
    
    volatile int loop_bound = N; // Volatile to prevent constant propagation
    
    if (use_offload) {
        // Complex nested OpenMP construct with SIMD clause
        // This should trigger the SIMT transformation for GPU offloading
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(from: a[0:N]) map(tofrom: result) \
            num_teams(4) thread_limit(128)
        for (int i = 0; i < loop_bound; i++) {
            // Non-trivial loop body with device function call
            int val = device_compute(a[i], b[i]);
            
            // Data-dependent operation
            if (val > THRESHOLD) {
                val = THRESHOLD;
            }
            
            // Reduction-like operation
            #pragma omp atomic
            result += val;
            
            a[i] = val * 2 - b[i];
        }
    } else {
        // Host fallback with similar structure
        host_version(a, b, &result);
    }
    
    // Compute checksum to ensure execution
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum ^= a[i];
    }
    
    printf("Result: %d\n", result);
    printf("Checksum: %d\n", checksum);
    
    // Additional test with nested parallel region inside target
    if (use_offload) {
        int partial_results[4] = {0};
        
        #pragma omp target teams distribute map(tofrom: partial_results[0:4]) \
            map(to: a[0:N])
        for (int team = 0; team < 4; team++) {
            #pragma omp parallel for simd reduction(+:partial_results[team])
            for (int i = team * (N/4); i < (team + 1) * (N/4); i++) {
                partial_results[team] += a[i] % 17;
            }
        }
        
        int final_sum = 0;
        for (int i = 0; i < 4; i++) {
            final_sum += partial_results[i];
        }
        printf("Final sum from nested: %d\n", final_sum);
    }
    
    free(a);
    free(b);
    
    return 0;
}
