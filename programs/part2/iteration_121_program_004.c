#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void compute_kernel(float *a, float *b, float *c, float scale, int n) {
    #pragma omp teams distribute parallel for simd collapse(2)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 16; ++j) {
            int idx = i * 16 + j;
            c[idx] = a[idx] + b[idx] * scale;
        }
    }
}

void reduction_kernel(float *data, float *result, int n, float threshold) {
    float sum = 0.0f;
    #pragma omp teams distribute parallel for simd reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        if (data[i] > threshold) {
            sum += data[i];
        }
    }
    *result = sum;
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    // Use command-line arguments to prevent constant propagation
    int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
    }
    
    // Use volatile to prevent optimization
    volatile int vol_N = N;
    int actual_N = vol_N;
    
    // Allocate arrays with dynamic size
    int total_size = actual_N * 16;
    float *a = (float*)malloc(total_size * sizeof(float));
    float *b = (float*)malloc(total_size * sizeof(float));
    float *c = (float*)malloc(total_size * sizeof(float));
    float *d = (float*)malloc(total_size * sizeof(float));
    
    // Initialize arrays
    for (int i = 0; i < total_size; ++i) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(total_size - i) * 0.05f;
        c[i] = 0.0f;
        d[i] = (float)i * 0.02f;
    }
    
    // Non-constant scale factor
    float scale = (argc > 2) ? atof(argv[2]) : 2.5f;
    float threshold = (argc > 3) ? atof(argv[3]) : 50.0f;
    
    // First target region with teams distribute parallel for simd
    #pragma omp target data map(to: a[0:total_size], b[0:total_size]) \
                            map(from: c[0:total_size])
    {
        #pragma omp target teams distribute parallel for simd collapse(2)
        for (int i = 0; i < actual_N; ++i) {
            for (int j = 0; j < 16; ++j) {
                int idx = i * 16 + j;
                // Complex enough to prevent optimization
                c[idx] = a[idx] + b[idx] * scale + (float)(i % 8) * 0.1f;
            }
        }
        
        // Second target region - different loop structure
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < total_size; ++i) {
            // Conditional computation to create more complex CFG
            if (i % 3 == 0) {
                c[i] = c[i] * 0.5f;
            } else if (i % 3 == 1) {
                c[i] = c[i] + a[i];
            } else {
                c[i] = c[i] - b[i];
            }
        }
    }
    
    // Third target region using declare target function
    float reduction_result = 0.0f;
    #pragma omp target data map(to: d[0:total_size]) \
                            map(from: reduction_result)
    {
        #pragma omp target 
        {
            reduction_kernel(d, &reduction_result, total_size, threshold);
        }
    }
    
    // Fourth target region - another variation
    #pragma omp target data map(to: a[0:total_size], b[0:total_size]) \
                            map(tofrom: c[0:total_size])
    {
        #pragma omp target 
        {
            compute_kernel(a, b, c, scale * 0.5f, actual_N);
        }
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < total_size; ++i) {
        checksum += c[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Reduction result: %f\n", reduction_result);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
