#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Declare target functions for offloading
#pragma omp declare target
void init_array(float *arr, int n, float val) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        arr[i] = val * i;
    }
}

float compute_scale(int n) {
    return (n % 2 == 0) ? 2.0f : 1.5f;
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    // Use volatile and command-line args to prevent constant folding
    volatile int base_size = 1000;
    int n = base_size;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = base_size;
    }
    
    // Dynamic sizes to prevent optimization
    int rows = n;
    int cols = (n % 10) + 5;  // Non-constant expression
    
    // Allocate arrays
    float *a = (float*)malloc(rows * cols * sizeof(float));
    float *b = (float*)malloc(rows * cols * sizeof(float));
    float *c = (float*)malloc(rows * cols * sizeof(float));
    float *d = (float*)malloc(rows * cols * sizeof(float));
    
    // Initialize on host
    for (int i = 0; i < rows * cols; ++i) {
        a[i] = 1.0f;
        b[i] = 2.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // Use volatile variable for scale to prevent constant propagation
    volatile float host_scale = 3.0f;
    float scale = host_scale;
    
    // First target region with teams distribute parallel for simd
    // Using collapse(2) for 2D loop to increase complexity
    #pragma omp target data map(to: a[0:rows*cols], b[0:rows*cols]) \
                            map(tofrom: c[0:rows*cols]) \
                            map(from: d[0:rows*cols])
    {
        // First SIMT loop - vector addition with scale
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(4) thread_limit(128)
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                int idx = i * cols + j;
                c[idx] = a[idx] + b[idx] * scale;
            }
        }
        
        // Call device function to modify scale
        float device_scale = compute_scale(rows);
        
        // Second SIMT loop - different computation pattern
        // Using simdlen(8) clause to encourage SIMT transformation
        #pragma omp target teams distribute parallel for simd \
                simdlen(8) num_teams(2)
        for (int i = 0; i < rows * cols; ++i) {
            d[i] = c[i] * device_scale - a[i];
            
            // Add conditional to prevent simple vectorization
            if (i % 7 == 0) {
                d[i] += 1.0f;
            }
        }
        
        // Third loop - reduction pattern
        float sum = 0.0f;
        #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) collapse(2)
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                int idx = i * cols + j;
                sum += d[idx] * (i + 1);
            }
        }
        
        // Use the result to prevent dead code elimination
        #pragma omp target update from(sum)
        printf("Intermediate sum: %f\n", sum);
    }
    
    // Additional target region with different data mapping
    {
        float *temp = (float*)malloc(rows * sizeof(float));
        
        #pragma omp target data map(to: c[0:rows*cols]) \
                                map(alloc: temp[0:rows])
        {
            // Fourth SIMT loop - 1D with stride
            #pragma omp target teams distribute parallel for simd \
                    num_teams(8)
            for (int i = 0; i < rows; ++i) {
                temp[i] = 0.0f;
                for (int j = 0; j < cols; j += 2) {
                    temp[i] += c[i * cols + j];
                }
            }
            
            #pragma omp target update from(temp[0:rows])
        }
        
        // Compute checksum on host
        float checksum = 0.0f;
        for (int i = 0; i < rows; ++i) {
            checksum += temp[i];
        }
        printf("Checksum: %f\n", checksum);
        
        free(temp);
    }
    
    // Final verification
    float final_sum = 0.0f;
    for (int i = 0; i < rows * cols; ++i) {
        final_sum += c[i] + d[i];
    }
    printf("Final sum: %f\n", final_sum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
