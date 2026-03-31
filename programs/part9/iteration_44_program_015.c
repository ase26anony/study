#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

// Variant 1: SIMD target loop
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step, 
                      volatile int n, volatile int m) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end], b[start:end]) map(from: c[start:end]) \
        collapse(2) num_teams(8) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            if (idx >= start && idx < end) {
                c[idx] = a[idx] * step + b[idx] / (step + 1);
            }
        }
    }
}

// Variant 2: Parallel target loop without SIMD
void parallel_target_loop(float *x, float *y, float scale, 
                         int low, int high, int stride,
                         volatile int limit) {
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high:stride]) map(tofrom: y[low:high:stride]) \
        private(scale) firstprivate(low, high) shared(stride)
    for (int i = low; i < high; i += stride) {
        if (i < limit) {
            y[i] = x[i] * scale + (float)i / (scale + 1.0f);
        }
    }
}

// Variant 3: Combined constructs with data region
void combined_constructs(double *arr1, double *arr2, double *result,
                        int size, int offset, volatile int mod) {
    const int chunk = size / 4;
    static double static_buffer[256];
    
    #pragma omp target data map(to: arr1[0:size], arr2[offset:size-offset]) \
                            map(tofrom: result[0:size])
    {
        // Initialize static buffer on device
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: static_buffer[0:256])
        for (int i = 0; i < 256; i++) {
            static_buffer[i] = (double)(i % mod);
        }
        
        // Main computation with complex iteration space
        #pragma omp target teams distribute parallel for simd \
            collapse(2) num_teams(4)
        for (int i = 0; i < size; i += chunk) {
            for (int j = 0; j < chunk; j++) {
                int idx = i + j;
                if (idx < size) {
                    double temp = arr1[idx] * arr2[idx + offset];
                    result[idx] = temp + static_buffer[idx % 256] * (double)j;
                }
            }
        }
    }
}

// Host-only parallel region for conditional execution
void host_only_parallel(int *data, int n, int factor) {
    #pragma omp parallel for simd schedule(static, 16)
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * factor + i;
    }
}

// Function to select between target and host execution
void conditional_execution(int *a, int *b, int *c, int size, 
                          int use_target, volatile int seed) {
    if (use_target) {
        // Force compiler to consider SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
            if(target: use_target > 0)
        for (int i = 0; i < size; i++) {
            c[i] = a[i] + b[i] * (seed % 7);
        }
    } else {
        host_only_parallel(c, size, seed % 5);
    }
}

int main(int argc, char *argv[]) {
    // Use command-line argument for seed if provided
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    printf("Running with seed: %d\n", seed);
    
    // Declare arrays with different storage durations
    static int static_array[N * M];
    int auto_array[N * M];
    const int const_size = N * M;
    volatile int vol_bound = N;
    
    // Initialize arrays with random data
    for (int i = 0; i < N * M; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = rand() % 100;
    }
    
    // Additional arrays for different types
    float float_array[N];
    double double_array[N];
    int result_array[N * M];
    
    for (int i = 0; i < N; i++) {
        float_array[i] = (float)rand() / RAND_MAX;
        double_array[i] = (double)rand() / RAND_MAX;
    }
    
    memset(result_array, 0, sizeof(result_array));
    
    // Main loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        printf("\nIteration %d:\n", iter);
        
        // Vary parameters using random values
        int use_simd = (iter % 2 == 0) ? 1 : 0;
        int start_idx = rand() % (N * M / 2);
        int end_idx = start_idx + (rand() % (N * M / 4)) + 100;
        int step = (rand() % 10) + 1;
        
        // Read volatile variables to prevent constant folding
        int current_n = vol_bound + (iter % 3);
        int current_m = M - (iter % 2);
        
        // Call variant functions with different patterns
        if (iter % 3 == 0) {
            printf("  Calling simd_target_loop...\n");
            simd_target_loop(static_array, auto_array, result_array,
                           start_idx, end_idx, step, current_n, current_m);
        } else if (iter % 3 == 1) {
            printf("  Calling parallel_target_loop...\n");
            int low = rand() % (N / 2);
            int high = low + (rand() % (N / 2)) + 50;
            int stride = (rand() % 3) + 1;
            float scale = (float)(rand() % 100) / 10.0f;
            
            parallel_target_loop(float_array, float_array, scale,
                               low, high, stride, N);
        } else {
            printf("  Calling combined_constructs...\n");
            int offset = rand() % 100;
            int mod = (rand() % 50) + 10;
            
            combined_constructs(double_array, double_array, double_array,
                              N, offset, mod);
        }
        
        // Conditional execution based on random value
        printf("  Calling conditional_execution...\n");
        int use_target = (rand() % 2) && (iter % 2 == 0);
        conditional_execution(static_array, auto_array, result_array,
                            N * M / 4, use_target, seed + iter);
        
        // Verify results with checksum
        long long checksum = 0;
        #pragma omp parallel for reduction(+:checksum)
        for (int i = 0; i < N * M; i++) {
            checksum += result_array[i];
        }
        
        printf("  Checksum: %lld\n", checksum);
        
        // Modify arrays for next iteration
        #pragma omp parallel for simd
        for (int i = 0; i < N * M; i++) {
            static_array[i] = (static_array[i] + iter) % 1000;
            auto_array[i] = (auto_array[i] * (iter + 1)) % 1000;
        }
    }
    
    // Final verification
    printf("\nFinal verification:\n");
    int final_check = 0;
    #pragma omp target teams distribute parallel for reduction(+:final_check) \
        map(to: result_array[0:N*M/2])
    for (int i = 0; i < N * M / 2; i++) {
        final_check += result_array[i] % 256;
    }
    
    printf("Final check value: %d\n", final_check);
    
    return 0;
}
