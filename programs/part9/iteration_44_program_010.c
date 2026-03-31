#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *restrict a, int *restrict b, int *restrict c,
                      int start, int end, int stride, volatile int n) {
    static int static_counter = 0;
    const int chunk_size = 64;
    int local_private = static_counter++;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) firstprivate(local_private) \
        private(chunk_size) shared(static_counter)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < chunk_size; j++) {
            int idx = i * stride + j + start;
            if (idx < end) {
                // Complex indexing to prevent optimization
                c[idx] = a[idx] * (i + local_private) + b[idx] * (j + static_counter);
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *restrict x, float *restrict y, 
                         float *restrict z, int low, int high,
                         volatile float scale) {
    float local_scale = scale;
    const float pi = 3.14159f;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high], y[low:high]) map(from: z[low:high]) \
        firstprivate(local_scale, pi)
    for (int i = low; i < high; i++) {
        // Non-linear computation to prevent optimization
        z[i] = x[i] * local_scale * (i % 32) + 
               y[i] * pi * ((i >> 3) & 0x7);
    }
}

/* Variant 3: Combined constructs with data region */
void combined_constructs(double *restrict d1, double *restrict d2,
                        double *restrict d3, int size, volatile int mode) {
    double *tmp = (double*)malloc(size * sizeof(double));
    if (!tmp) return;
    
    #pragma omp target data map(to: d1[0:size], d2[0:size]) \
                            map(from: d3[0:size]) map(alloc: tmp[0:size])
    {
        // Initialize tmp with pointer arithmetic
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < size; i++) {
            *(tmp + i) = *(d1 + i) - *(d2 + i);
        }
        
        // Conditional execution path
        if (mode & 0x1) {
            #pragma omp target teams distribute parallel for simd \
                collapse(2)
            for (int i = 0; i < size/2; i++) {
                for (int j = 0; j < 2; j++) {
                    int idx = i * 2 + j;
                    d3[idx] = tmp[idx] * (i + j) + d1[idx] * d2[idx];
                }
            }
        } else {
            #pragma omp target teams distribute parallel for
            for (int i = 0; i < size; i++) {
                d3[i] = tmp[i] * i + d1[i] * d2[i];
            }
        }
    }
    
    free(tmp);
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int n, volatile int factor) {
    int local_factor = factor;
    
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * local_factor + (i % 16);
    }
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    // Mixed storage duration arrays
    static int static_array[N];
    int auto_array[N];
    const int const_size = M;
    float float_array[M];
    double double_array[N];
    
    // Initialize with random data
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = rand() % 100;
        if (i < M) float_array[i] = (float)(rand() % 100) / 10.0f;
        double_array[i] = (double)(rand() % 100) / 5.0;
    }
    
    // Volatile variables to prevent constant folding
    volatile int v_n = N;
    volatile float v_scale = 2.5f;
    volatile int v_mode = 0;
    
    long total_checksum = 0;
    
    for (int iter = 0; iter < MAX_ITER; iter++) {
        // Vary parameters each iteration
        int start = rand() % (N/4);
        int end = N - rand() % (N/4);
        int stride = 1 + (iter % 3);
        
        // Randomly choose between target and host execution
        if (rand() % 2) {
            printf("Iteration %d: Using target regions\n", iter);
            
            // Call variant functions with different constructs
            simd_target_loop(static_array, auto_array, auto_array,
                           start, end, stride, v_n + iter);
            
            parallel_target_loop(float_array, float_array, float_array,
                               iter * 10, const_size, v_scale * (iter + 1));
            
            v_mode = iter % 4;
            combined_constructs(double_array, double_array, double_array,
                              N - iter * 50, v_mode);
        } else {
            printf("Iteration %d: Using host-only parallel\n", iter);
            host_only_parallel(auto_array, v_n, iter + 1);
        }
        
        // Compute checksum to prevent dead code elimination
        long checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum += static_array[i] + auto_array[i];
            if (i < M) checksum += (long)(float_array[i] * 100);
            checksum += (long)(double_array[i] * 100);
        }
        
        total_checksum += checksum;
        printf("  Checksum: %ld\n", checksum);
        
        // Modify some data for next iteration
        for (int i = 0; i < N; i += 32) {
            static_array[i] += iter;
            auto_array[i] -= iter;
            if (i < M) float_array[i] *= 1.1f;
            double_array[i] /= 1.05;
        }
    }
    
    printf("Total checksum: %ld\n", total_checksum);
    printf("Seed used: %d\n", seed);
    
    return 0;
}
