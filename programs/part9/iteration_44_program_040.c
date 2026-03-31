#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

// Variant 1: SIMD target loop with complex data environment
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step, 
                      volatile int n, volatile int m) {
    static int static_counter = 0;
    const int chunk_size = 32;
    int firstprivate_val = static_counter++;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) map(to: a[start:end:step], b[start:end:step]) \
        map(from: c[start:end:step]) firstprivate(firstprivate_val) \
        private(chunk_size) shared(static_counter)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            if (idx >= start && idx < end && (idx - start) % step == 0) {
                c[idx] = a[idx] + b[idx] * firstprivate_val + 
                         (i % chunk_size) - (j % chunk_size);
            }
        }
    }
}

// Variant 2: Parallel target loop without SIMD clause
void parallel_target_loop(float *x, float *y, float *z, int low, int high, 
                          int stride, volatile int limit) {
    float local_accum = 0.0f;
    const float scale = 2.5f;
    
    #pragma omp target data map(to: x[low:high:stride]) \
        map(tofrom: y[low:high:stride]) map(from: z[low:high:stride])
    {
        #pragma omp target teams distribute parallel for \
            num_teams(8) thread_limit(128) reduction(+:local_accum)
        for (int i = low; i < high; i += stride) {
            z[i] = x[i] * scale + y[i];
            y[i] = z[i] / scale;
            local_accum += z[i];
        }
    }
    
    // Use result to prevent optimization
    if (local_accum > limit) {
        printf("Accum: %.2f\n", local_accum);
    }
}

// Variant 3: Combined constructs with pointer arithmetic
void combined_constructs(double *p, double *q, double *r, int offset, 
                         int length, volatile int mode) {
    double *p_slice = p + offset;
    double *q_slice = q + offset;
    double *r_slice = r + offset;
    
    int use_simd = (mode % 3 == 0);
    
    if (use_simd) {
        #pragma omp target teams distribute parallel for simd \
            map(to: p_slice[0:length], q_slice[0:length]) \
            map(from: r_slice[0:length]) if(target:omp_get_num_devices()>0)
        for (int i = 0; i < length; i++) {
            // Complex indexing to prevent optimization
            int idx = (i * 17) % length;
            r_slice[idx] = p_slice[i] * q_slice[(i + 1) % length] - 
                          p_slice[(i + length - 1) % length] * q_slice[i];
        }
    } else {
        #pragma omp target teams distribute parallel for \
            map(to: p_slice[0:length], q_slice[0:length]) \
            map(from: r_slice[0:length])
        for (int i = 0; i < length; i++) {
            r_slice[i] = p_slice[i] + q_slice[i];
        }
    }
}

// Host-only parallel region for conditional execution
void host_only_parallel(int *arr, int size, volatile int factor) {
    #pragma omp parallel for simd schedule(static, 16)
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * factor + i;
    }
}

int main(int argc, char *argv[]) {
    // Initialize with command-line argument for variability
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    // Volatile variables to prevent constant folding
    volatile int v_n = N + (rand() % 100);
    volatile int v_m = M + (rand() % 50);
    volatile int total_size = v_n * v_m;
    
    // Arrays with different storage durations
    static int static_arr[N * M];
    int auto_arr[N * M];
    int *heap_arr = (int *)malloc(total_size * sizeof(int));
    float *float_arr1 = (float *)malloc(total_size * sizeof(float));
    float *float_arr2 = (float *)malloc(total_size * sizeof(float));
    float *float_arr3 = (float *)malloc(total_size * sizeof(float));
    double *double_arr1 = (double *)malloc(total_size * sizeof(double));
    double *double_arr2 = (double *)malloc(total_size * sizeof(double));
    double *double_arr3 = (double *)malloc(total_size * sizeof(double));
    
    // Initialize arrays with random data
    for (int i = 0; i < total_size; i++) {
        static_arr[i] = rand() % 100;
        auto_arr[i] = rand() % 100;
        heap_arr[i] = rand() % 100;
        float_arr1[i] = (float)rand() / RAND_MAX;
        float_arr2[i] = (float)rand() / RAND_MAX;
        float_arr3[i] = 0.0f;
        double_arr1[i] = (double)rand() / RAND_MAX;
        double_arr2[i] = (double)rand() / RAND_MAX;
        double_arr3[i] = 0.0;
    }
    
    // Main loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int mode = rand() % 4;
        volatile int start = rand() % (total_size / 2);
        volatile int end = start + 100 + (rand() % 200);
        volatile int step = 1 + (rand() % 3);
        volatile int offset = rand() % 100;
        volatile int length = 200 + (rand() % 300);
        
        // Conditional execution to influence SIMT transformation
        if (mode == 0 || mode == 2) {
            // Call target region functions
            simd_target_loop(static_arr, auto_arr, heap_arr, 
                           start, end, step, v_n, v_m);
            
            parallel_target_loop(float_arr1, float_arr2, float_arr3,
                               offset, offset + length, 2, length);
        } else {
            // Call host-only function
            host_only_parallel(heap_arr, total_size / 2, mode);
        }
        
        // Always call combined constructs
        combined_constructs(double_arr1, double_arr2, double_arr3,
                          offset, length, mode);
        
        // Verify results with checksum
        int int_sum = 0;
        float float_sum = 0.0f;
        double double_sum = 0.0;
        
        #pragma omp parallel for reduction(+:int_sum, float_sum, double_sum)
        for (int i = 0; i < length; i++) {
            if (i + offset < total_size) {
                int_sum += heap_arr[i + offset];
                float_sum += float_arr3[i + offset];
                double_sum += double_arr3[i + offset];
            }
        }
        
        printf("Iter %d: int_sum=%d, float_sum=%.2f, double_sum=%.2f\n",
               iter, int_sum, float_sum, double_sum);
        
        // Modify arrays for next iteration
        #pragma omp simd
        for (int i = 0; i < total_size; i++) {
            static_arr[i] = (static_arr[i] + iter) % 1000;
            auto_arr[i] = (auto_arr[i] * 3) % 1000;
        }
    }
    
    // Cleanup
    free(heap_arr);
    free(float_arr1);
    free(float_arr2);
    free(float_arr3);
    free(double_arr1);
    free(double_arr2);
    free(double_arr3);
    
    return 0;
}
