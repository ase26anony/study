#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step) {
    volatile int v_start = start;
    volatile int v_end = end;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[v_start:v_end], b[v_start:v_end]) \
        map(from: c[v_start:v_end]) \
        firstprivate(v_start, v_end, step) \
        private(i) \
        num_teams(4) thread_limit(128)
    for (int i = v_start; i < v_end; i += step) {
        c[i] = a[i] * step + b[i];
    }
}

/* Variant 2: Parallel target loop without SIMD */
void parallel_target_loop(float *x, float *y, float *z, int low, int high, float scale) {
    static const float const_factor = 2.5f;
    volatile int v_low = low;
    volatile int v_high = high;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[v_low:v_high], y[v_low:v_high]) \
        map(from: z[v_low:v_high]) \
        firstprivate(v_low, v_high, scale, const_factor) \
        collapse(2)
    for (int i = v_low; i < v_high; i++) {
        for (int j = 0; j < 8; j++) {
            int idx = i * 8 + j;
            if (idx < v_high * 8) {
                z[idx] = x[idx] * scale + y[idx] * const_factor;
            }
        }
    }
}

/* Variant 3: Combined constructs with data region */
void combined_constructs(double *p, double *q, double *r, int n, int m, int offset) {
    volatile int v_n = n;
    volatile int v_m = m;
    int *temp = (int*)malloc(v_n * sizeof(int));
    
    #pragma omp target data map(to: p[0:v_n], q[0:v_n]) \
                            map(from: r[0:v_n]) \
                            map(alloc: temp[0:v_n/2])
    {
        #pragma omp target teams distribute parallel for simd \
            firstprivate(v_n, v_m, offset) \
            private(i) \
            num_teams(v_m/64) thread_limit(256)
        for (int i = 0; i < v_n; i++) {
            double t = p[i] * q[i];
            r[i] = t + (i % v_m) * offset;
            
            if (i < v_n/2) {
                temp[i] = (int)(t * 100);
            }
        }
        
        // Additional computation on host
        #pragma omp parallel for simd
        for (int i = 0; i < v_n/2; i++) {
            r[i] += temp[i] * 0.01;
        }
    }
    
    free(temp);
}

/* Variant 4: Function with conditional target execution */
void conditional_target(int *arr1, int *arr2, int size, int use_target) {
    volatile int v_size = size;
    int *local_copy = (int*)malloc(v_size * sizeof(int));
    memcpy(local_copy, arr1, v_size * sizeof(int));
    
    if (use_target) {
        #pragma omp target teams distribute parallel for simd \
            map(to: arr2[0:v_size]) \
            map(tofrom: local_copy[0:v_size]) \
            firstprivate(v_size)
        for (int i = 0; i < v_size; i++) {
            local_copy[i] = local_copy[i] * 2 + arr2[i];
        }
    } else {
        #pragma omp parallel for simd
        for (int i = 0; i < v_size; i++) {
            local_copy[i] = local_copy[i] * 3 + arr2[i];
        }
    }
    
    memcpy(arr1, local_copy, v_size * sizeof(int));
    free(local_copy);
}

/* Host-only parallel region for comparison */
void host_only_parallel(double *data, int n, int iter) {
    volatile int v_n = n;
    volatile int v_iter = iter;
    
    #pragma omp parallel for simd
    for (int i = 0; i < v_n; i++) {
        for (int j = 0; j < v_iter; j++) {
            data[i] = data[i] * 1.1 + j * 0.01;
        }
    }
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    // Declare arrays with different storage durations
    static int static_array[N];
    int auto_array[N];
    const int const_size = M;
    volatile int vol_bound = N/2;
    
    float float_array[N];
    double double_array[N];
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        static_array[i] = i;
        auto_array[i] = rand() % 100;
        float_array[i] = (float)rand() / RAND_MAX;
        double_array[i] = (double)rand() / RAND_MAX;
    }
    
    int result_array[N];
    float result_float[N];
    double result_double[N];
    
    // Clear result arrays
    memset(result_array, 0, N * sizeof(int));
    memset(result_float, 0, N * sizeof(float));
    memset(result_double, 0, N * sizeof(double));
    
    printf("Starting OpenMP SIMT transformation test with seed %d\n", seed);
    
    for (int iter = 0; iter < MAX_ITER; iter++) {
        // Vary parameters to create different transformation contexts
        int use_target = (iter % 2 == 0) ? 1 : 0;
        int start_idx = rand() % (N/4);
        int end_idx = N/2 + rand() % (N/4);
        int step = 1 + (iter % 3);
        float scale = 1.0f + (float)iter * 0.5f;
        int offset = iter * 10;
        
        // Call variant functions with different patterns
        if (iter % 3 == 0) {
            simd_target_loop(static_array, auto_array, result_array, 
                           start_idx, end_idx, step);
            
            // Verify results
            int checksum = 0;
            for (int i = start_idx; i < end_idx; i += step) {
                checksum += result_array[i];
            }
            printf("Iter %d, simd_target checksum: %d\n", iter, checksum);
        }
        else if (iter % 3 == 1) {
            parallel_target_loop(float_array, float_array, result_float,
                               start_idx/2, end_idx/2, scale);
            
            // Verify results
            float fchecksum = 0.0f;
            for (int i = start_idx/2; i < end_idx/2; i++) {
                fchecksum += result_float[i];
            }
            printf("Iter %d, parallel_target checksum: %.2f\n", iter, fchecksum);
        }
        else {
            combined_constructs(double_array, double_array, result_double,
                              const_size, M/4, offset);
            
            // Verify results
            double dchecksum = 0.0;
            for (int i = 0; i < const_size; i++) {
                dchecksum += result_double[i];
            }
            printf("Iter %d, combined_constructs checksum: %.2f\n", iter, dchecksum);
        }
        
        // Mix with conditional target execution
        conditional_target(auto_array, static_array, vol_bound, use_target);
        
        // Occasionally call host-only version
        if (iter % 4 == 0) {
            host_only_parallel(result_double, const_size, iter+1);
        }
        
        // Vary array sections using pointer arithmetic
        int *arr_section = auto_array + start_idx;
        int *res_section = result_array + start_idx;
        
        #pragma omp target teams distribute parallel for simd \
            map(to: arr_section[0:end_idx-start_idx]) \
            map(from: res_section[0:end_idx-start_idx]) \
            firstprivate(start_idx, end_idx)
        for (int i = 0; i < end_idx - start_idx; i++) {
            res_section[i] = arr_section[i] * (i + start_idx);
        }
    }
    
    // Final verification
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += result_array[i] + (int)result_float[i] + (int)result_double[i];
    }
    printf("Final combined checksum: %d\n", final_sum);
    
    return 0;
}
