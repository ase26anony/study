#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step,
                      volatile int bound1, volatile int bound2) {
    static int static_counter = 0;
    const int const_offset = 10;
    int local_private = static_counter++;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) map(to: a[start:end:step], b[start:end:step]) \
        map(from: c[start:end:step]) firstprivate(local_private) \
        private(end) shared(static_counter)
    for (int i = bound1; i < bound2; i++) {
        for (int j = 0; j < M/2; j++) {
            int idx = i * (M/2) + j;
            if (idx >= start && idx < end && idx % step == 0) {
                c[idx] = a[idx] + b[idx] * (local_private + const_offset) 
                         + (i % 2 == 0 ? 1 : -1) * j;
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, 
                          int low, int high, int stride,
                          volatile int dyn_bound) {
    float *ptr_x = &x[low];
    float *ptr_y = &y[low];
    float *ptr_z = &z[low];
    int firstprivate_val = low % 10;
    
    #pragma omp target teams distribute parallel for \
        map(to: ptr_x[0:high-low:stride], ptr_y[0:high-low:stride]) \
        map(from: ptr_z[0:high-low:stride]) \
        firstprivate(firstprivate_val, low, high, stride)
    for (int i = 0; i < dyn_bound; i++) {
        int actual_idx = low + i * stride;
        if (actual_idx < high) {
            ptr_z[i] = ptr_x[i] * ptr_y[i] + 
                      (float)firstprivate_val * sinf((float)i * 0.1f);
        }
    }
}

/* Variant 3: Combined constructs with data region */
void combined_constructs(double *mat1, double *mat2, double *result,
                         int rows, int cols, volatile int seed) {
    const int tile_size = 32;
    int *dynamic_array = (int*)malloc(rows * sizeof(int));
    
    for (int i = 0; i < rows; i++) {
        dynamic_array[i] = (i * seed) % 100;
    }
    
    #pragma omp target data map(to: mat1[0:rows*cols], mat2[0:rows*cols], \
                                       dynamic_array[0:rows]) \
                            map(from: result[0:rows*cols])
    {
        #pragma omp target teams distribute parallel for simd \
            collapse(2) firstprivate(tile_size, seed)
        for (int i = 0; i < rows; i += tile_size) {
            for (int j = 0; j < cols; j += tile_size) {
                for (int ii = i; ii < i + tile_size && ii < rows; ii++) {
                    for (int jj = j; jj < j + tile_size && jj < cols; jj++) {
                        int idx = ii * cols + jj;
                        result[idx] = mat1[idx] * mat2[idx] + 
                                     (double)dynamic_array[ii] * 
                                     (jj % 2 == 0 ? 1.0 : -1.0);
                    }
                }
            }
        }
    }
    
    free(dynamic_array);
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int size, volatile int factor) {
    int local_sum = 0;
    
    #pragma omp parallel for reduction(+:local_sum) \
        if(factor > 0) num_threads(factor % 4 + 1)
    for (int i = 0; i < size; i++) {
        arr[i] = (arr[i] * factor) % 1000;
        local_sum += arr[i];
    }
    
    printf("Host-only sum: %d\n", local_sum);
}

/* Function that conditionally calls target or host regions */
void conditional_region_execution(int *data, int size, 
                                  int use_target, volatile int threshold) {
    if (use_target && threshold > size/2) {
        int *temp = (int*)malloc(size * sizeof(int));
        memcpy(temp, data, size * sizeof(int));
        
        #pragma omp target teams distribute parallel for simd \
            map(to: temp[0:size]) map(from: data[0:size])
        for (int i = 0; i < size; i++) {
            data[i] = temp[i] * 2 + (i % threshold);
        }
        
        free(temp);
    } else {
        host_only_parallel(data, size, threshold);
    }
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Declare arrays with different storage durations */
    static int static_array[N];
    int auto_array[N];
    volatile int volatile_bound = N;
    const int const_size = M;
    
    float float_array[N];
    double double_matrix[N * M/2];
    double double_matrix2[N * M/2];
    double result_matrix[N * M/2];
    
    /* Initialize arrays with random/semi-random data */
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = i + rand() % 10;
        float_array[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    
    for (int i = 0; i < N * M/2; i++) {
        double_matrix[i] = (double)rand() / RAND_MAX * 50.0;
        double_matrix2[i] = (double)rand() / RAND_MAX * 30.0;
    }
    
    printf("Starting OpenMP SIMT transformation test with seed %d\n", seed);
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int dynamic_bound = (rand() % (N/2)) + N/4;
        volatile int step = (rand() % 3) + 1;
        int start = iter * (N/MAX_ITER);
        int end = start + (N/MAX_ITER);
        
        printf("\nIteration %d: bound=%d, step=%d, start=%d, end=%d\n",
               iter, dynamic_bound, step, start, end);
        
        /* Call variant functions with different patterns */
        if (iter % 3 == 0) {
            simd_target_loop(static_array, auto_array, auto_array,
                            start, end, step, 0, dynamic_bound);
        } else if (iter % 3 == 1) {
            parallel_target_loop(float_array, float_array, float_array,
                                start, end, step, dynamic_bound);
        } else {
            combined_constructs(double_matrix, double_matrix2, result_matrix,
                               N/4, M/2, dynamic_bound);
        }
        
        /* Conditional execution based on runtime values */
        int use_target_region = (rand() % 2) && (iter > 0);
        conditional_region_execution(auto_array, N/8, 
                                    use_target_region, dynamic_bound);
        
        /* Compute checksums to prevent dead code elimination */
        int int_sum = 0;
        float float_sum = 0.0f;
        double double_sum = 0.0;
        
        #pragma omp parallel for reduction(+:int_sum, float_sum, double_sum) \
            if(iter % 2 == 0)
        for (int i = 0; i < N/16; i++) {
            int idx = i * 16;
            if (idx < N) {
                int_sum += static_array[idx] + auto_array[idx];
                float_sum += float_array[idx];
            }
            if (i < N * M/32) {
                double_sum += result_matrix[i];
            }
        }
        
        printf("Checksums - int: %d, float: %.2f, double: %.2f\n",
               int_sum, float_sum, double_sum);
        
        /* Modify volatile bound for next iteration */
        volatile_bound = (volatile_bound * 13 + 7) % N;
    }
    
    /* Final verification with array sections */
    int final_check = 0;
    #pragma omp target teams distribute parallel for reduction(+:final_check) \
        map(to: static_array[0:N:2]) if(N > 500)
    for (int i = 0; i < N/2; i++) {
        final_check += static_array[i*2];
    }
    
    printf("\nFinal check sum: %d\n", final_check);
    
    return 0;
}
