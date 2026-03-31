#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void device_init(float* arr, int n, float val) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        arr[i] = val;
    }
}
#pragma omp end declare target

int main(int argc, char** argv) {
    // Use volatile and command-line args to prevent constant propagation
    volatile int base_n = 1000;
    int n = base_n;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = base_n;
    }
    
    // Dynamic scaling factor to prevent optimizations
    float scale = 2.5f;
    if (argc > 2) {
        scale = atof(argv[2]);
    }
    
    // Allocate arrays with dynamic sizes
    float* a = (float*)malloc(n * sizeof(float));
    float* b = (float*)malloc(n * sizeof(float));
    float* c = (float*)malloc(n * sizeof(float));
    float* d = (float*)malloc(n * sizeof(float));
    
    // Initialize host arrays
    for (int i = 0; i < n; ++i) {
        a[i] = (float)i;
        b[i] = (float)(n - i);
    }
    
    // Offload region 1: Teams distribute parallel for simd with collapse
    #pragma omp target data map(to: a[0:n], b[0:n]) map(from: c[0:n])
    {
        // First SIMT loop - 2D collapse to increase complexity
        #pragma omp target teams distribute parallel for simd collapse(2) \
            num_teams(4) thread_limit(128)
        for (int i = 0; i < n/2; ++i) {
            for (int j = 0; j < 2; ++j) {
                int idx = i * 2 + j;
                if (idx < n) {
                    c[idx] = a[idx] + b[idx] * scale;
                }
            }
        }
        
        // Call device function to initialize array d
        #pragma omp target map(from: d[0:n])
        {
            device_init(d, n, 1.0f);
        }
        
        // Second SIMT loop - different pattern with conditional
        #pragma omp target teams distribute parallel for simd \
            num_teams(8) thread_limit(64)
        for (int i = 0; i < n; ++i) {
            // Data-dependent computation to prevent optimization
            if (c[i] > 0.0f) {
                d[i] = d[i] * c[i] - scale;
            } else {
                d[i] = d[i] + c[i] / scale;
            }
        }
    }
    
    // Third offload region - separate data mapping
    float* e = (float*)malloc(n * sizeof(float));
    #pragma omp target data map(to: c[0:n]) map(from: e[0:n])
    {
        // Another SIMT loop with reduction-like pattern
        float local_sum = 0.0f;
        #pragma omp target teams distribute parallel for simd \
            reduction(+:local_sum) num_teams(2)
        for (int i = 0; i < n; ++i) {
            e[i] = c[i] * d[i];
            local_sum += e[i];
        }
        
        // Use the result to prevent dead code elimination
        volatile float sum_check = local_sum;
        (void)sum_check;
    }
    
    // Compute checksum on host
    float checksum = 0.0f;
    for (int i = 0; i < n; ++i) {
        checksum += c[i] + d[i] + e[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Array sizes: %d\n", n);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    
    return 0;
}
