#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void device_init(float *arr, int n, float val) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        arr[i] = val;
    }
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    // Use volatile and command-line args to prevent constant propagation
    volatile int base_size = 1000;
    int N = base_size;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = base_size;
    }
    
    // Dynamic scaling factor
    float scale = 2.5f;
    if (argc > 2) {
        scale = atof(argv[2]);
    }
    
    // Allocate arrays with dynamic sizes
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *d = (float*)malloc(N * sizeof(float));
    
    // Initialize host arrays
    for (int i = 0; i < N; ++i) {
        a[i] = i * 1.0f;
        b[i] = i * 0.5f;
    }
    
    // Offload data to device
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N], d[0:N])
    {
        // FIRST TARGET REGION: Teams distribute parallel for simd
        // This should trigger SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(128)
        for (int i = 0; i < N; ++i) {
            c[i] = a[i] + b[i] * scale;
        }
        
        // Initialize d on device using declare target function
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < N; ++i) {
            d[i] = 0.0f;
        }
        
        // SECOND TARGET REGION: 2D collapsed loop
        // Increases chance of hitting transformation with different pattern
        int M = (N > 100) ? 100 : N;
        #pragma omp target teams distribute parallel for simd collapse(2) \
            num_teams(2) thread_limit(64)
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < M; ++j) {
                int idx = i * M + j;
                if (idx < N) {
                    d[idx] = c[idx] * 2.0f - a[idx];
                }
            }
        }
        
        // THIRD TARGET REGION: Conditional computation
        // Different control flow pattern
        volatile float threshold = 500.0f;  // volatile prevents constant folding
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < N; ++i) {
            if (c[i] > threshold) {
                d[i] = d[i] / scale;
            } else {
                d[i] = d[i] * scale;
            }
        }
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < N; ++i) {
        checksum += c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Array sizes: %d\n", N);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
