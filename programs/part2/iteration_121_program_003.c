#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void device_init(float *arr, int n, float val) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        arr[i] = val * i;
    }
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    // Use command-line arguments to prevent constant propagation
    int N = 10000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 10000;
    }
    
    // Use volatile to prevent optimization
    volatile int dynamic_N = N;
    int actual_N = dynamic_N;
    
    // Allocate arrays
    float *a = (float*)malloc(actual_N * sizeof(float));
    float *b = (float*)malloc(actual_N * sizeof(float));
    float *c = (float*)malloc(actual_N * sizeof(float));
    float *d = (float*)malloc(actual_N * sizeof(float));
    
    // Initialize on host
    for (int i = 0; i < actual_N; ++i) {
        a[i] = i * 1.5f;
        b[i] = i * 2.5f;
    }
    
    // Non-constant scale factor
    float scale = (argc > 2) ? atof(argv[2]) : 3.14f;
    
    // First target region with teams distribute parallel for simd
    #pragma omp target data map(to: a[0:actual_N], b[0:actual_N]) \
                            map(from: c[0:actual_N])
    {
        // This should trigger SIMT transformation
        #pragma omp target teams distribute parallel for simd \
                num_teams(actual_N/256) thread_limit(256)
        for (int i = 0; i < actual_N; ++i) {
            c[i] = a[i] + b[i] * scale;
        }
        
        // Call device function to initialize array d
        #pragma omp target data map(alloc: d[0:actual_N])
        {
            #pragma omp target teams distribute
            for (int i = 0; i < 1; ++i) {
                device_init(d, actual_N, 2.0f);
            }
        }
    }
    
    // Second target region with 2D collapsed loop to increase complexity
    int M = (actual_N > 100) ? 100 : actual_N;
    int K = actual_N / M;
    
    #pragma omp target data map(tofrom: c[0:actual_N]) map(to: d[0:actual_N])
    {
        // Collapsed 2D loop - more likely to trigger complex SIMT handling
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(K) thread_limit(128)
        for (int i = 0; i < K; ++i) {
            for (int j = 0; j < M; ++j) {
                int idx = i * M + j;
                if (idx < actual_N) {
                    c[idx] = c[idx] * 0.5f + d[idx] * scale;
                }
            }
        }
    }
    
    // Third region with conditional computation
    float threshold = (argc > 3) ? atof(argv[3]) : 1000.0f;
    
    #pragma omp target data map(tofrom: c[0:actual_N])
    {
        #pragma omp target teams distribute parallel for simd \
                num_teams(actual_N/512) thread_limit(512)
        for (int i = 0; i < actual_N; ++i) {
            if (c[i] > threshold) {
                c[i] = c[i] / threshold;
            } else {
                c[i] = c[i] * threshold;
            }
        }
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < actual_N; ++i) {
        checksum += c[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Array size: %d\n", actual_N);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
