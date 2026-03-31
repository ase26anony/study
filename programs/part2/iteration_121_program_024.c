#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
float compute_scale(float x) {
    return x * 2.0f;
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    // Use command-line arguments to prevent constant folding
    int N = 10000;
    if (argc > 1) {
        N = atoi(argv[1]);
    }
    
    // Use volatile to prevent optimization
    volatile int dynamic_N = N;
    
    // Allocate and initialize arrays
    float *a = (float*)malloc(dynamic_N * sizeof(float));
    float *b = (float*)malloc(dynamic_N * sizeof(float));
    float *c = (float*)malloc(dynamic_N * sizeof(float));
    float *d = (float*)malloc(dynamic_N * sizeof(float));
    
    for (int i = 0; i < dynamic_N; i++) {
        a[i] = (float)i;
        b[i] = (float)(dynamic_N - i);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // Non-constant scale factor
    float scale = 2.5f;
    if (argc > 2) {
        scale = atof(argv[2]);
    }
    
    // First target region with teams distribute parallel for simd
    #pragma omp target data map(to: a[0:dynamic_N], b[0:dynamic_N]) \
                            map(from: c[0:dynamic_N])
    {
        // This should trigger SIMT transformation
        #pragma omp target teams distribute parallel for simd \
                num_teams(4) thread_limit(128)
        for (int i = 0; i < dynamic_N; i++) {
            c[i] = a[i] + b[i] * scale;
        }
        
        // Second loop with different pattern - increases chance of transformation
        #pragma omp target teams distribute parallel for simd \
                collapse(2) num_teams(8)
        for (int i = 0; i < dynamic_N/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < dynamic_N) {
                    c[idx] += compute_scale(c[idx]);
                }
            }
        }
    }
    
    // Second target region with different data mapping
    #pragma omp target data map(to: c[0:dynamic_N]) \
                            map(from: d[0:dynamic_N])
    {
        // Another SIMT candidate with conditional inside
        #pragma omp target teams distribute parallel for simd \
                num_teams(2)
        for (int i = 0; i < dynamic_N; i++) {
            float val = c[i];
            if (val > 1000.0f) {
                d[i] = val * 0.5f;
            } else {
                d[i] = val * 2.0f;
            }
        }
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < dynamic_N; i++) {
        checksum += d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Array size: %d\n", dynamic_N);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
