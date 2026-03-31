#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void device_compute(float *a, float *b, float *c, int n, float scale) {
    // Empty - just to ensure declare target region
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    // Use command-line argument to prevent constant folding
    int N = 10000;
    if (argc > 1) {
        N = atoi(argv[1]);
    }
    
    // Use volatile to prevent optimization
    volatile int volatile_N = N;
    int actual_N = volatile_N;
    
    // Allocate arrays
    float *a = (float*)malloc(actual_N * sizeof(float));
    float *b = (float*)malloc(actual_N * sizeof(float));
    float *c = (float*)malloc(actual_N * sizeof(float));
    float *d = (float*)malloc(actual_N * sizeof(float));
    
    // Initialize arrays
    for (int i = 0; i < actual_N; i++) {
        a[i] = (float)i;
        b[i] = (float)(actual_N - i);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // Non-constant scale factor
    float scale = 2.5f;
    if (argc > 2) {
        scale = atof(argv[2]);
    }
    
    // First target region with teams distribute parallel for simd
    // This should trigger the SIMT transformation
    #pragma omp target data map(to: a[0:actual_N], b[0:actual_N]) map(from: c[0:actual_N])
    {
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(4) thread_limit(128)
        for (int i = 0; i < actual_N/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < actual_N) {
                    c[idx] = a[idx] + b[idx] * scale;
                }
            }
        }
        
        // Second target region with different computation pattern
        // Increases chance of hitting the transformation block again
        #pragma omp target teams distribute parallel for simd \
                num_teams(8) thread_limit(64)
        for (int i = 0; i < actual_N; i++) {
            // Conditional computation to create more complex control flow
            if (i % 3 == 0) {
                c[i] = c[i] * 0.5f;
            } else if (i % 3 == 1) {
                c[i] = c[i] + a[i];
            } else {
                c[i] = c[i] - b[i];
            }
        }
    }
    
    // Third target region with reduction-like pattern
    #pragma omp target data map(to: a[0:actual_N]) map(tofrom: d[0:actual_N])
    {
        #pragma omp target teams distribute parallel for simd \
                num_teams(2) thread_limit(256)
        for (int i = 0; i < actual_N; i++) {
            float temp = 0.0f;
            for (int k = 0; k < 4; k++) {
                temp += a[(i + k) % actual_N] * (k + 1);
            }
            d[i] = temp;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    float checksum = 0.0f;
    for (int i = 0; i < actual_N; i++) {
        checksum += c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Array sizes: %d\n", actual_N);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
