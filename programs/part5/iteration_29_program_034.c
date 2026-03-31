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

void host_version(float *a, float *b, int n, int *result) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        int temp = (int)(a[i] * 100) + (int)(b[i] * 100);
        sum += device_compute(temp, i);
    }
    *result = sum;
}

int main(int argc, char *argv[]) {
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i % 37) * 0.1f;
        b[i] = (i % 23) * 0.2f;
    }
    
    int result = 0;
    int use_offload = 0;
    
    // Runtime condition to decide execution path
    if (argc > 1 && strcmp(argv[1], "offload") == 0) {
        use_offload = 1;
    }
    
    if (use_offload) {
        // Complex nested OpenMP offloading with SIMD
        // This should trigger the SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(from: result) \
            reduction(+:result) num_teams(4) thread_limit(128)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop body with device function call
            int temp = (int)(a[i] * 100) + (int)(b[i] * 100);
            result += device_compute(temp, i);
            
            // Additional computation to create substantial GIMPLE
            if (i % 2 == 0) {
                result += (temp % 7);
            } else {
                result -= (temp % 5);
            }
        }
    } else {
        // Host fallback version
        host_version(a, b, N, &result);
    }
    
    // Conditional execution with another layer
    int final_result = 0;
    if (result > THRESHOLD) {
        // Nested parallel region with SIMD inside target
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: final_result) map(to: a[0:N/2]) \
            num_teams(2)
        for (int i = 0; i < N/2; i++) {
            #pragma omp simd
            for (int j = 0; j < 4; j++) {
                final_result += (int)(a[i] * (j + 1));
            }
        }
    } else {
        final_result = result;
    }
    
    printf("Result: %d\n", final_result);
    printf("Checksum: %ld\n", (long)(final_result + N));
    
    // Cleanup
    free(a);
    free(b);
    
    return 0;
}
