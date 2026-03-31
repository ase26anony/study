#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

// Variant 1: SIMD target loop with complex data environment
void simd_target_loop(int *a, int *b, int *c, int start, int end, 
                      int stride, volatile int n, int thread_limit) {
    static int static_counter = 0;
    const int chunk_size = 64;
    int private_var = start;
    
    #pragma omp target teams distribute parallel for simd \
        num_teams(thread_limit) thread_limit(256) \
        collapse(2) private(private_var) firstprivate(start) \
        map(to: a[start:end:stride], b[start:end:stride]) \
        map(tofrom: c[start:end:stride])
    for (int i = start; i < end; i += stride) {
        for (int j = 0; j < chunk_size; j++) {
            private_var = i + j;
            int idx = i + (j % stride);
            if (idx < end) {
                c[idx] = a[idx] * private_var + b[idx] / (private_var + 1);
                static_counter++;  // Static variable access
            }
        }
    }
}

// Variant 2: Parallel target loop without SIMD clause
void parallel_target_loop(float *x, float *y, float *z, 
                         int low, int high, volatile int m) {
    float local_sum = 0.0f;
    float *ptr = z + low;
    
    #pragma omp target teams distribute parallel for \
        num_teams(m/64) map(to: x[low:high], y[low:high]) \
        map(tofrom: z[low:high]) reduction(+:local_sum)
    for (int i = low; i < high; i++) {
        ptr = z + i;
        *ptr = x[i] * 2.5f + y[i] / 1.7f;
        
        // Complex indexing with pointer arithmetic
        for (int k = 0; k < 4; k++) {
            if ((i + k) < high) {
                z[i + k] += 0.1f * k;
            }
        }
        local_sum += z[i];
    }
}

// Variant 3: Combined constructs with data region
void combined_constructs(double *d1, double *d2, double *result,
                        int size, int offset, volatile int iter) {
    const double scale = 3.14159;
    double *temp = (double*)malloc(size * sizeof(double));
    
    #pragma omp target data map(to: d1[0:size], d2[offset:size-offset]) \
                            map(from: temp[0:size])
    {
        #pragma omp target teams distribute parallel for simd \
            num_teams(iter) thread_limit(128) \
            map(alloc: result[0:size])
        for (int i = 0; i < size; i++) {
            double t = d1[i] * scale;
            for (int j = 0; j < 2; j++) {
                int idx = (i + j) % size;
                temp[idx] = t + d2[(idx + offset) % size];
            }
            result[i] = temp[i] * (i % 8 + 1);
        }
    }
    
    free(temp);
}

// Host-only parallel region for conditional execution
void host_only_parallel(int *arr, int n, volatile int seed) {
    int local_seed = seed;
    
    #pragma omp parallel for simd schedule(dynamic, 16) \
        private(local_seed) firstprivate(n)
    for (int i = 0; i < n; i++) {
        local_seed = (local_seed * 1103515245 + 12345) & 0x7fffffff;
        arr[i] = (local_seed % 100) + i;
    }
}

// Function to select between target and host execution
void conditional_execution(int *data, int size, int use_target, 
                          volatile int param) {
    if (use_target && param > 0) {
        int *copy = (int*)malloc(size * sizeof(int));
        memcpy(copy, data, size * sizeof(int));
        
        #pragma omp target teams distribute parallel for simd \
            map(to: copy[0:size]) map(from: data[0:size])
        for (int i = 0; i < size; i++) {
            data[i] = copy[i] * param + i;
        }
        
        free(copy);
    } else {
        host_only_parallel(data, size, param);
    }
}

int main(int argc, char *argv[]) {
    // Use command-line argument for random seed
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    // Declare arrays with different storage durations
    static int static_array[N];
    int auto_array[N];
    volatile int vol_size = N;
    const int const_size = M;
    
    float float_array[N];
    double double_array[N];
    
    // Initialize arrays with varying patterns
    for (int i = 0; i < N; i++) {
        static_array[i] = i * 2;
        auto_array[i] = rand() % 100;
        float_array[i] = (float)rand() / RAND_MAX * 100.0f;
        double_array[i] = (double)rand() / RAND_MAX * 200.0;
    }
    
    int result_array[N];
    memset(result_array, 0, N * sizeof(int));
    
    printf("Starting OpenMP SIMT transformation test with seed %d\n", seed);
    
    // Main loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int dynamic_bound = (rand() % (N/2)) + (N/4);
        int use_simd = (iter % 2 == 0);
        int thread_limit = (rand() % 8 + 1) * 32;
        
        printf("\nIteration %d: bound=%d, simd=%d, threads=%d\n", 
               iter, dynamic_bound, use_simd, thread_limit);
        
        // Call variant functions with different parameters
        if (use_simd) {
            simd_target_loop(static_array, auto_array, result_array,
                           0, dynamic_bound, 2, vol_size, thread_limit);
        } else {
            parallel_target_loop(float_array, float_array + M/2, 
                               float_array, 0, dynamic_bound, const_size);
        }
        
        // Combined constructs with offset
        int offset = (iter * 17) % (N/4);
        combined_constructs(double_array, double_array + offset,
                          double_array, dynamic_bound, offset, iter + 1);
        
        // Conditional execution based on random value
        int use_target = (rand() % 3 == 0);
        conditional_execution(auto_array, dynamic_bound, 
                            use_target, iter * 7 + seed);
        
        // Compute and verify checksums
        int int_sum = 0;
        float float_sum = 0.0f;
        double double_sum = 0.0;
        
        #pragma omp parallel for reduction(+:int_sum, float_sum, double_sum) \
            simd
        for (int i = 0; i < dynamic_bound; i++) {
            int_sum += result_array[i];
            float_sum += float_array[i];
            double_sum += double_array[i];
        }
        
        printf("  Checksums: int=%d, float=%.2f, double=%.2f\n",
               int_sum, float_sum, double_sum);
        
        // Force memory synchronization
        #pragma omp target update from(result_array[0:dynamic_bound]) \
            if(use_simd)
    }
    
    // Final verification with complex loop
    volatile int final_check = 0;
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: final_check) reduction(+:final_check) \
        if(seed % 2 == 0)
    for (int i = 0; i < N; i += 4) {
        for (int j = 0; j < 4; j++) {
            if (i + j < N) {
                final_check += static_array[i + j] * auto_array[i + j];
            }
        }
    }
    
    printf("\nFinal check value: %d\n", final_check);
    return 0;
}
