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

float device_compute(float a, float b, float scale) {
    return a + b * scale;
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    // Use command-line argument to prevent constant propagation
    int N = (argc > 1) ? atoi(argv[1]) : 10000;
    if (N <= 0) N = 10000;
    
    // Use volatile to prevent optimization
    volatile int dynamic_N = N;
    int actual_N = dynamic_N;
    
    // Allocate arrays
    float *a = (float*)malloc(actual_N * sizeof(float));
    float *b = (float*)malloc(actual_N * sizeof(float));
    float *c = (float*)malloc(actual_N * sizeof(float));
    float *d = (float*)malloc(actual_N * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize on host
    for (int i = 0; i < actual_N; ++i) {
        a[i] = 1.0f;
        b[i] = 2.0f * i;
    }
    
    // Non-constant scale to prevent folding
    float scale = (argc > 2) ? atof(argv[2]) : 3.14f;
    volatile float volatile_scale = scale;
    
    // First target region with teams distribute parallel for simd
    #pragma omp target data map(to: a[0:actual_N], b[0:actual_N]) \
                            map(from: c[0:actual_N])
    {
        // This should trigger SIMT transformation
        #pragma omp target teams distribute parallel for simd \
                num_teams(4) thread_limit(128) collapse(2)
        for (int i = 0; i < actual_N/2; ++i) {
            for (int j = 0; j < 2; ++j) {
                int idx = i * 2 + j;
                if (idx < actual_N) {
                    c[idx] = device_compute(a[idx], b[idx], volatile_scale);
                }
            }
        }
        
        // Second similar region with different computation pattern
        #pragma omp target teams distribute parallel for simd \
                num_teams(8) thread_limit(64)
        for (int i = 0; i < actual_N; ++i) {
            // Conditional computation to create more complex CFG
            if (i % 3 == 0) {
                c[i] = c[i] * 2.0f - a[i];
            } else {
                c[i] = c[i] + b[i] / volatile_scale;
            }
        }
    }
    
    // Third region with reduction-like pattern
    float sum = 0.0f;
    #pragma omp target data map(to: c[0:actual_N]) map(tofrom: sum)
    {
        #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) collapse(2)
        for (int i = 0; i < actual_N/4; ++i) {
            for (int j = 0; j < 4; ++j) {
                int idx = i * 4 + j;
                if (idx < actual_N) {
                    sum += c[idx];
                    d[idx] = c[idx] * (i + 1);
                }
            }
        }
    }
    
    // Fourth region: nested parallelism with simd on innermost
    #pragma omp target data map(tofrom: d[0:actual_N])
    {
        #pragma omp target teams distribute parallel for \
                num_teams(2) num_threads(32)
        for (int block = 0; block < 4; ++block) {
            int start = block * (actual_N/4);
            int end = (block + 1) * (actual_N/4);
            if (end > actual_N) end = actual_N;
            
            #pragma omp simd
            for (int i = start; i < end; ++i) {
                d[i] = d[i] + (i % 10) * 0.1f;
            }
        }
    }
    
    // Verify computation
    float checksum = 0.0f;
    for (int i = 0; i < actual_N; ++i) {
        checksum += c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Sum from reduction: %f\n", sum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
