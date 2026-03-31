#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

// Variant 1: SIMD target loop with complex data environment
void simd_target_loop(int *a, int *b, int *c, int start, int end, int stride, 
                      volatile int n, volatile int m) {
    static int static_counter = 0;
    const int chunk_size = 32;
    int local_n = n;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) firstprivate(local_n, chunk_size) \
        private(static_counter)
    for (int i = 0; i < local_n; i += chunk_size) {
        for (int j = 0; j < m; j++) {
            int idx = (i * m + j) % (end - start);
            c[start + idx] = a[start + idx] + b[start + idx] * (i + j);
        }
    }
}

// Variant 2: Parallel target loop without SIMD clause
void parallel_target_loop(float *x, float *y, float *z, int low, int high, 
                          volatile float scale, volatile int offset) {
    float local_scale = scale;
    int local_offset = offset;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high], y[low:high]) map(from: z[low:high]) \
        firstprivate(local_scale, local_offset)
    for (int i = low; i < high; i++) {
        z[i] = x[i] * local_scale + y[i] + local_offset;
        // Add conditional to create branching
        if (i % 16 == 0) {
            z[i] *= 2.0f;
        }
    }
}

// Variant 3: Combined constructs with data region
void combined_constructs(double *p, double *q, double *r, int size, 
                         volatile double alpha, volatile double beta) {
    const int block = 64;
    double *temp = (double*)malloc(size * sizeof(double));
    
    #pragma omp target data map(to: p[0:size], q[0:size]) \
                            map(from: r[0:size]) map(alloc: temp[0:size])
    {
        #pragma omp target teams distribute parallel for simd \
            firstprivate(alpha, beta, block)
        for (int i = 0; i < size; i += block) {
            int limit = (i + block < size) ? i + block : size;
            for (int j = i; j < limit; j++) {
                temp[j] = p[j] * alpha + q[j] * beta;
                // Complex indexing with pointer arithmetic
                double *ptr = &r[j];
                *ptr = temp[j] + (j % 8) * 0.125;
            }
        }
    }
    
    free(temp);
}

// Host-only parallel region for conditional execution
void host_only_parallel(int *arr, int n, volatile int factor) {
    int local_factor = factor;
    
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * local_factor + i;
    }
}

// Function that selects between target and host execution
void conditional_execution(int *data, int size, int use_target, 
                          volatile int param) {
    if (use_target) {
        int *copy1 = (int*)malloc(size * sizeof(int));
        int *copy2 = (int*)malloc(size * sizeof(int));
        
        memcpy(copy1, data, size * sizeof(int));
        memcpy(copy2, data, size * sizeof(int));
        
        // Use array section with stride
        simd_target_loop(data, copy1, copy2, 0, size, 2, size/2, size/4);
        
        memcpy(data, copy2, size * sizeof(int));
        
        free(copy1);
        free(copy2);
    } else {
        host_only_parallel(data, size, param);
    }
}

int main(int argc, char *argv[]) {
    // Use command-line argument for seed
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    // Declare arrays with different storage durations
    static int static_array[N];
    int auto_array[N];
    const int const_size = N;
    volatile int vol_bound = M;
    
    // Initialize arrays with random data
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = rand() % 100;
    }
    
    float float_array[M];
    double double_array[N];
    for (int i = 0; i < M; i++) {
        float_array[i] = (float)rand() / RAND_MAX;
    }
    for (int i = 0; i < N; i++) {
        double_array[i] = (double)rand() / RAND_MAX;
    }
    
    // Main loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        // Vary parameters using random numbers and iteration index
        volatile int use_simt = (rand() % 3) > 0;  // Mostly use SIMT
        volatile int offset = rand() % 100;
        volatile float scale = 1.0f + (float)(rand() % 100) / 100.0f;
        volatile double alpha = 0.5 + (double)(rand() % 100) / 200.0;
        volatile double beta = 0.3 + (double)(rand() % 100) / 300.0;
        
        // Call variant functions with different array slices
        int start = iter * (N / MAX_ITER);
        int end = (iter + 1) * (N / MAX_ITER);
        
        printf("Iteration %d: start=%d, end=%d, use_simt=%d\n", 
               iter, start, end, use_simt);
        
        // Mix of function calls to expose different contexts
        if (iter % 2 == 0) {
            simd_target_loop(static_array, auto_array, static_array, 
                           start, end, 1, vol_bound, const_size);
        } else {
            conditional_execution(auto_array, end - start, 
                                use_simt, offset);
        }
        
        parallel_target_loop(float_array, float_array, float_array,
                           0, M/2, scale, offset);
        
        combined_constructs(double_array, double_array, double_array,
                          N, alpha, beta);
        
        // Compute checksums to prevent elimination
        int int_sum = 0;
        float float_sum = 0.0f;
        double double_sum = 0.0;
        
        #pragma omp parallel for reduction(+:int_sum, float_sum, double_sum)
        for (int i = 0; i < N; i++) {
            if (i < N) int_sum += static_array[i] + auto_array[i];
            if (i < N) double_sum += double_array[i];
            if (i < M) float_sum += float_array[i];
        }
        
        printf("  Checksums: int=%d, float=%.2f, double=%.2f\n",
               int_sum, float_sum, double_sum);
    }
    
    return 0;
}
