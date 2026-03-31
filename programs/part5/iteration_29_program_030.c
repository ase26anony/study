#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
float device_func(float x, float y) {
    return x * y + (x - y);
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float result = 0.0f;
    
    // Initialize arrays with non-constant patterns
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 37) % 50) * 0.2f;
    }
    
    // Use argc to create runtime condition for offloading decision
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // This should trigger the SIMT transformation for GPU offloading
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(from: result) reduction(+:result)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop with device function call
            float temp = device_func(a[i], b[i]);
            result += temp * (i % 10 + 1); // Data-dependent computation
        }
    } else {
        // Host fallback version
        #pragma omp parallel for simd reduction(+:result)
        for (int i = 0; i < N; i++) {
            float temp = a[i] * b[i] + (a[i] - b[i]);
            result += temp * (i % 10 + 1);
        }
    }
    
    // Additional nested OpenMP construct to increase complexity
    float final_result = 0.0f;
    #pragma omp parallel
    {
        #pragma omp for simd reduction(+:final_result)
        for (int i = 0; i < N/2; i++) {
            final_result += result * 0.01f * (i % 5 + 1);
        }
    }
    
    printf("Result: %f\n", final_result);
    
    // Simple checksum to prevent optimization removal
    volatile float checksum = final_result;
    if (checksum > 1000000.0f) {
        printf("Unexpected large result\n");
    }
    
    free(a);
    free(b);
    
    return 0;
}
