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
                      volatile int bound1, volatile int bound2) {
    static int static_counter = 0;
    const int local_const = 100;
    int private_var = 0;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) map(to: a[start:end:step], b[start:end:step]) \
        map(from: c[start:end:step]) private(private_var) \
        firstprivate(local_const) shared(static_counter)
    for (int i = bound1; i < bound2; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            private_var = local_const + (i % 10);
            c[idx] = a[idx] * private_var + b[idx] / (local_const + 1);
            #pragma omp atomic
            static_counter++;
        }
    }
}

// Variant 2: Parallel target loop without SIMD clause
void parallel_target_loop(float *x, float *y, float *z, int low, int high, 
                         int stride, volatile int n_iter) {
    float local_accum = 0.0f;
    const float scale = 2.5f;
    
    #pragma omp target data map(to: x[low:high:stride], y[low:high:stride]) \
                            map(tofrom: z[low:high:stride])
    {
        #pragma omp target teams distribute parallel for \
            private(local_accum) firstprivate(scale) reduction(+:local_accum)
        for (int i = 0; i < n_iter; i++) {
            int idx = low + i * stride;
            if (idx < high) {
                local_accum += x[idx] * scale;
                z[idx] = y[idx] + local_accum / (i + 1);
            }
        }
    }
}

// Variant 3: Combined constructs with pointer arithmetic
void combined_constructs(double *p, double *q, double *r, int offset, 
                        int length, volatile int mode) {
    double *slice_p = p + offset;
    double *slice_q = q + offset;
    double *slice_r = r + offset;
    
    #pragma omp target data map(to: slice_p[0:length], slice_q[0:length]) \
                            map(from: slice_r[0:length])
    {
        if (mode % 2 == 0) {
            #pragma omp target teams distribute parallel for simd \
                collapse(2) schedule(static, 16)
            for (int i = 0; i < length / 64; i++) {
                for (int j = 0; j < 64; j++) {
                    int idx = i * 64 + j;
                    slice_r[idx] = slice_p[idx] * slice_q[idx] + 
                                  (double)(i + j) / (double)length;
                }
            }
        } else {
            #pragma omp target teams distribute parallel for
            for (int i = 0; i < length; i++) {
                slice_r[i] = slice_p[i] / (slice_q[i] + 1.0) + 
                            (double)(offset + i) * 0.01;
            }
        }
    }
}

// Host-only parallel region for conditional execution
void host_only_parallel(int *arr, int size, volatile int factor) {
    #pragma omp parallel for simd
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * factor + i;
    }
}

int main(int argc, char *argv[]) {
    // Initialize with command-line seed for variability
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    // Declare arrays with mixed storage durations
    static int static_array[N * M];
    int auto_array[N * M];
    float float_array[N * M];
    double double_array[N * M];
    
    // Initialize arrays with random data
    for (int i = 0; i < N * M; i++) {
        static_array[i] = rand() % 1000;
        auto_array[i] = rand() % 1000;
        float_array[i] = (float)(rand() % 1000) / 10.0f;
        double_array[i] = (double)(rand() % 1000) / 10.0;
    }
    
    // Results arrays
    int result_int[N * M];
    float result_float[N * M];
    double result_double[N * M];
    memset(result_int, 0, sizeof(result_int));
    memset(result_float, 0, sizeof(result_float));
    memset(result_double, 0, sizeof(result_double));
    
    // Volatile variables to prevent constant folding
    volatile int v_bound1 = rand() % 100 + 50;
    volatile int v_bound2 = v_bound1 + rand() % 200 + 100;
    volatile int v_stride = (rand() % 5 + 1) * 2;
    volatile int v_mode = rand() % 4;
    
    // Main execution loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        printf("Iteration %d:\n", iter);
        
        // Randomly choose between target and host execution
        int use_target = (rand() % 3) > 0;
        
        if (use_target) {
            // Call SIMD target variant
            int start = rand() % 100;
            int end = start + rand() % 500 + 100;
            simd_target_loop(static_array, auto_array, result_int, 
                           start, end, v_stride, v_bound1, v_bound2);
            
            // Calculate checksum
            long long checksum1 = 0;
            for (int i = start; i < end; i += v_stride) {
                checksum1 += result_int[i];
            }
            printf("  SIMD target checksum: %lld\n", checksum1);
            
            // Call parallel target variant
            int low = rand() % 200;
            int high = low + rand() % 300 + 100;
            volatile int n_iter = rand() % 100 + 50;
            parallel_target_loop(float_array, float_array + N*M/2, 
                               result_float, low, high, 2, n_iter);
            
            // Calculate checksum
            double checksum2 = 0.0;
            for (int i = low; i < high; i += 2) {
                checksum2 += result_float[i];
            }
            printf("  Parallel target checksum: %.2f\n", checksum2);
            
            // Call combined constructs variant
            int offset = rand() % 100;
            int length = rand() % 400 + 200;
            combined_constructs(double_array, double_array + N*M/4,
                              result_double, offset, length, v_mode);
            
            // Calculate checksum
            double checksum3 = 0.0;
            for (int i = 0; i < length; i++) {
                checksum3 += result_double[offset + i];
            }
            printf("  Combined constructs checksum: %.2f\n", checksum3);
        } else {
            // Host-only execution path
            volatile int factor = rand() % 10 + 1;
            host_only_parallel(result_int, N * M / 2, factor);
            
            // Calculate checksum
            long long checksum = 0;
            for (int i = 0; i < N * M / 2; i++) {
                checksum += result_int[i];
            }
            printf("  Host-only checksum: %lld\n", checksum);
        }
        
        // Modify volatile variables for next iteration
        v_bound1 += rand() % 10;
        v_bound2 += rand() % 20;
        v_mode = (v_mode + 1) % 4;
        
        printf("\n");
    }
    
    return 0;
}
