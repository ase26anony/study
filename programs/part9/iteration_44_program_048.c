#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define L 256

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step, 
                      volatile int bound1, volatile int bound2) {
    static int static_counter = 0;
    const int const_offset = 10;
    int local_private = static_counter++;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) map(to: a[start:end:step], b[start:end:step]) \
        map(from: c[start:end:step]) private(local_private) \
        firstprivate(const_offset) shared(static_counter)
    for (int i = 0; i < bound1; i++) {
        for (int j = 0; j < bound2; j++) {
            int idx = i * bound2 + j;
            if (idx < (end - start) / step) {
                int actual_idx = start + idx * step;
                c[actual_idx] = a[actual_idx] + b[actual_idx] * const_offset 
                               + (local_private % 16);
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, int low, int high, 
                         int stride, volatile int dyn_bound) {
    float *ptr_x = x + low;
    float *ptr_y = y + low;
    float *ptr_z = z + low;
    int length = (high - low) / stride;
    
    #pragma omp target data map(to: ptr_x[0:length:stride], ptr_y[0:length:stride]) \
                            map(from: ptr_z[0:length:stride])
    {
        #pragma omp target teams distribute parallel for \
            num_teams(dyn_bound % 8 + 1) thread_limit(128)
        for (int i = 0; i < length; i++) {
            int actual_idx = i * stride;
            ptr_z[actual_idx] = ptr_x[actual_idx] * 2.0f - ptr_y[actual_idx] / 3.0f;
        }
    }
}

/* Variant 3: Combined constructs with nested parallelism */
void combined_constructs(double *mat1, double *mat2, double *result, 
                        int rows, int cols, volatile int seed) {
    const int tile_size = 32;
    int *dynamic_sizes = (int*)malloc(rows * sizeof(int));
    
    /* Initialize with runtime-dependent values */
    for (int i = 0; i < rows; i++) {
        dynamic_sizes[i] = (seed + i) % cols + 1;
    }
    
    #pragma omp target data map(to: mat1[0:rows*cols], mat2[0:rows*cols], \
                                       dynamic_sizes[0:rows]) \
                            map(from: result[0:rows*cols])
    {
        #pragma omp target teams distribute parallel for simd \
            collapse(2) private(tile_size)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < dynamic_sizes[i]; j++) {
                int idx = i * cols + j;
                result[idx] = mat1[idx] * mat2[idx] + (double)(i * j) / 100.0;
            }
        }
    }
    
    free(dynamic_sizes);
}

/* Variant 4: Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int size, volatile int factor) {
    int local_sum = 0;
    
    #pragma omp parallel for reduction(+:local_sum) if(factor > 2)
    for (int i = 0; i < size; i++) {
        arr[i] = (arr[i] * factor) % 1000;
        local_sum += arr[i];
    }
    
    /* Use result to prevent optimization */
    arr[0] = local_sum % 1000;
}

/* Helper function with pointer arithmetic */
void process_slice(int *base, int offset, int length, int stride, 
                   volatile int modifier) {
    int *slice = base + offset;
    
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: slice[0:length:stride]) \
        if(modifier % 3 == 0)
    for (int i = 0; i < length; i++) {
        int idx = i * stride;
        slice[idx] = (slice[idx] + modifier) ^ 0xFF;
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for variability */
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Declare arrays with mixed storage durations */
    static int static_array[N];
    int auto_array[N];
    const int const_array[N] = {[0 ... N-1] = 1};
    volatile int volatile_bound = N;
    
    float float_array[M];
    double double_matrix[L][L];
    
    /* Initialize with runtime-dependent values */
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = (seed + i) % 100;
    }
    
    for (int i = 0; i < M; i++) {
        float_array[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            double_matrix[i][j] = (double)((i * j + seed) % 100) / 3.0;
        }
    }
    
    int result_array[N];
    float result_float[M];
    double result_matrix[L][L];
    
    /* Clear results */
    memset(result_array, 0, N * sizeof(int));
    memset(result_float, 0, M * sizeof(float));
    memset(result_matrix, 0, L * L * sizeof(double));
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < 5; iter++) {
        volatile int dynamic_bound = (rand() % 100) + 50;
        volatile int slice_start = rand() % (N/2);
        volatile int slice_stride = (rand() % 3) + 1;
        volatile int slice_end = slice_start + (rand() % 200) + 100;
        
        /* Conditional execution to influence transformation decisions */
        if (iter % 2 == 0) {
            /* Call target region variants */
            simd_target_loop(static_array, auto_array, result_array,
                           slice_start, slice_end, slice_stride,
                           dynamic_bound, dynamic_bound/2);
            
            parallel_target_loop(float_array, float_array, result_float,
                               0, M, 2, dynamic_bound);
            
            combined_constructs(&double_matrix[0][0], &double_matrix[0][0],
                              &result_matrix[0][0], L, L, seed + iter);
        } else {
            /* Call host-only variant */
            host_only_parallel(auto_array, N, dynamic_bound);
            
            /* Also call target variant but with different conditions */
            process_slice(static_array, slice_start, 
                         (slice_end - slice_start) / slice_stride,
                         slice_stride, dynamic_bound);
        }
        
        /* Compute checksums to prevent dead code elimination */
        int int_sum = 0;
        float float_sum = 0.0f;
        double double_sum = 0.0;
        
        #pragma omp parallel for reduction(+:int_sum) if(iter > 2)
        for (int i = 0; i < N; i++) {
            int_sum += result_array[i] % 256;
        }
        
        #pragma omp simd reduction(+:float_sum)
        for (int i = 0; i < M; i++) {
            float_sum += result_float[i];
        }
        
        for (int i = 0; i < L; i++) {
            for (int j = 0; j < L; j++) {
                double_sum += result_matrix[i][j];
            }
        }
        
        printf("Iteration %d: Checksums - int: %d, float: %.2f, double: %.2f\n",
               iter, int_sum, float_sum, double_sum);
        
        /* Modify bounds for next iteration */
        volatile_bound = (volatile_bound * 13 + 7) % 200;
    }
    
    return 0;
}
