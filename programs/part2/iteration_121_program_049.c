#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void device_compute(float* a, float* b, float* c, int n, float scale) {
    // Empty - just to mark function for offloading
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
    
    // Use volatile scale to prevent optimization
    volatile float vol_scale = 2.5f;
    float scale = vol_scale;
    
    // Allocate arrays with dynamic size
    float* a = (float*)malloc(n * sizeof(float));
    float* b = (float*)malloc(n * sizeof(float));
    float* c1 = (float*)malloc(n * sizeof(float));
    float* c2 = (float*)malloc(n * sizeof(float));
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        a[i] = (float)i;
        b[i] = (float)(n - i);
        c1[i] = 0.0f;
        c2[i] = 0.0f;
    }
    
    // Mark device function as used
    #pragma omp target data map(to: a[0:n], b[0:n]) map(from: c1[0:n], c2[0:n])
    {
        // FIRST TARGET REGION: teams distribute parallel for simd
        // This should trigger SIMT transformation with conditional wrapper
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(4) thread_limit(128)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < 4; j++) {  // Inner dimension for collapse(2)
                int idx = i * 4 + j;
                if (idx < n) {
                    c1[idx] = a[idx] + b[idx] * scale;
                }
            }
        }
        
        // SECOND TARGET REGION: Different pattern to potentially trigger again
        // Using simdlen clause to influence SIMT decisions
        float local_scale = scale * 0.5f;  // Different computation
        #pragma omp target teams distribute parallel for simd \
                simdlen(8) num_teams(2)
        for (int i = 0; i < n; i++) {
            // Conditional computation to prevent simple vectorization
            if (i % 2 == 0) {
                c2[i] = a[i] * local_scale - b[i];
            } else {
                c2[i] = b[i] * local_scale + a[i];
            }
        }
        
        // THIRD TARGET REGION: Reduction pattern
        float sum = 0.0f;
        #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) map(tofrom: sum)
        for (int i = 0; i < n; i++) {
            sum += c1[i] + c2[i];
        }
        
        printf("Reduction sum: %f\n", sum);
    }
    
    // Compute checksum on host to verify computation
    float checksum = 0.0f;
    for (int i = 0; i < n; i++) {
        checksum += c1[i] + c2[i];
    }
    printf("Host checksum: %f\n", checksum);
    
    // Cleanup
    free(a);
    free(b);
    free(c1);
    free(c2);
    
    return 0;
}
