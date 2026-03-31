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
                      volatile int bound, const int offset) {
    static int static_counter = 0;
    const int local_const = 100;
    int private_var = 0;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end:step], b[start:end:step]) \
        map(from: c[start:end:step]) \
        private(private_var) firstprivate(local_const) \
        collapse(2) num_teams(4) thread_limit(128)
    for (int i = 0; i < bound; i++) {
        for (int j = 0; j < M/2; j++) {
            private_var = i * j + local_const;
            int idx = i * (M/2) + j;
            if (idx < end - start) {
                c[start + idx] = a[start + idx] * b[start + idx] + 
                                private_var + offset + static_counter;
            }
        }
    }
    
    static_counter++;
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, 
                         int low, int high, int stride,
                         volatile float scale) {
    float local_accum = 0.0f;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high:stride], y[low:high:stride]) \
        map(tofrom: z[low:high:stride]) \
        reduction(+:local_accum) \
        num_teams(8)
    for (int i = low; i < high; i += stride) {
        float temp = x[i] * scale + y[i];
        z[i] = temp * temp;
        local_accum += z[i];
        
        /* Complex indexing to prevent optimization */
        if (i % 3 == 0) {
            z[i] += (float)(i & 0xFF);
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile float sink = local_accum;
    (void)sink;
}

/* Variant 3: Combined constructs with data region */
void combined_constructs(double *mat1, double *mat2, double *result,
                        int rows, int cols, volatile int iter) {
    double *tmp = (double*)malloc(rows * cols * sizeof(double));
    
    #pragma omp target data map(to: mat1[0:rows*cols], mat2[0:rows*cols]) \
                            map(alloc: tmp[0:rows*cols]) \
                            map(from: result[0:rows*cols])
    {
        /* Initialize temporary array */
        #pragma omp target teams distribute parallel for simd \
            map(always, tofrom: tmp[0:rows*cols])
        for (int i = 0; i < rows * cols; i++) {
            tmp[i] = mat1[i] + mat2[i] * (iter % 10);
        }
        
        /* Main computation with nested loops */
        #pragma omp target teams distribute parallel for simd \
            collapse(2) \
            map(to: tmp[0:rows*cols]) \
            map(from: result[0:rows*cols])
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                double sum = 0.0;
                for (int k = 0; k < cols; k++) {
                    sum += tmp[i * cols + k] * tmp[k * cols + j];
                }
                result[idx] = sum / (iter + 1);
            }
        }
    }
    
    free(tmp);
}

/* Variant 4: Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int size, volatile int factor) {
    int local_sum = 0;
    
    #pragma omp parallel for reduction(+:local_sum) \
        private(factor) /* Note: private on volatile - intentional complexity */
    for (int i = 0; i < size; i++) {
        int private_factor = factor;
        arr[i] = (arr[i] * private_factor + i) % 256;
        local_sum += arr[i];
    }
    
    volatile int sink = local_sum;
    (void)sink;
}

/* Function to select target vs host execution based on condition */
void conditional_execution(int *data, int size, int use_target, 
                          volatile int threshold) {
    if (use_target && threshold > 0) {
        /* Create artificial data dependencies */
        int *copy1 = (int*)malloc(size * sizeof(int));
        int *copy2 = (int*)malloc(size * sizeof(int));
        memcpy(copy1, data, size * sizeof(int));
        memcpy(copy2, data, size * sizeof(int));
        
        #pragma omp target teams distribute parallel for simd \
            map(to: copy1[0:size]) map(tofrom: copy2[0:size]) \
            map(from: data[0:size])
        for (int i = 0; i < size; i++) {
            data[i] = copy1[i] * copy2[i] + threshold;
        }
        
        free(copy1);
        free(copy2);
    } else {
        host_only_parallel(data, size, threshold);
    }
}

int main(int argc, char *argv[]) {
    /* Initialize with command-line argument for variability */
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Declare arrays with mixed storage durations */
    static int static_array[N];
    int auto_array[N];
    const int const_size = N;
    volatile int vol_bound = M;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < N; i++) {
        static_array[i] = i + seed;
        auto_array[i] = (i * 3) % 256;
    }
    
    /* Float arrays for different type testing */
    float float_arr1[M], float_arr2[M], float_result[M];
    for (int i = 0; i < M; i++) {
        float_arr1[i] = (float)(rand() % 1000) / 100.0f;
        float_arr2[i] = (float)(rand() % 1000) / 100.0f;
        float_result[i] = 0.0f;
    }
    
    /* Double arrays for matrix operations */
    int rows = 32, cols = 32;
    double *mat1 = (double*)malloc(rows * cols * sizeof(double));
    double *mat2 = (double*)malloc(rows * cols * sizeof(double));
    double *mat_result = (double*)malloc(rows * cols * sizeof(double));
    for (int i = 0; i < rows * cols; i++) {
        mat1[i] = (double)(rand() % 1000) / 10.0;
        mat2[i] = (double)(rand() % 1000) / 10.0;
        mat_result[i] = 0.0;
    }
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int dynamic_bound = (rand() % 100) + 50;
        volatile float scale_factor = (float)(rand() % 100) / 10.0f;
        int use_target = (rand() % 2);
        
        printf("Iteration %d: bound=%d, scale=%.2f, use_target=%d\n",
               iter, dynamic_bound, scale_factor, use_target);
        
        /* Call variant functions with different slices and parameters */
        int start = iter * 100;
        int end = start + dynamic_bound;
        int stride = (iter % 3) + 1;
        
        /* Variant 1: SIMD target loop */
        int *temp_result = (int*)malloc(N * sizeof(int));
        memcpy(temp_result, auto_array, N * sizeof(int));
        simd_target_loop(static_array, auto_array, temp_result, 
                        start, end, stride, dynamic_bound, iter);
        
        /* Verify and use results */
        int checksum1 = 0;
        for (int i = start; i < end; i += stride) {
            checksum1 += temp_result[i];
        }
        printf("  SIMD target checksum: %d\n", checksum1);
        free(temp_result);
        
        /* Variant 2: Parallel target loop */
        parallel_target_loop(float_arr1, float_arr2, float_result,
                           0, M, 2, scale_factor);
        
        /* Variant 3: Combined constructs */
        combined_constructs(mat1, mat2, mat_result, rows, cols, iter);
        
        /* Variant 4: Conditional execution */
        conditional_execution(auto_array, N / 2, use_target, dynamic_bound);
        
        /* Additional mixed usage to stress transformation */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: static_array[0:N/4]) \
            collapse(2)
        for (int i = 0; i < dynamic_bound/2; i++) {
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < N/4) {
                    static_array[idx] = static_array[idx] * 
                                       (i + j + iter) % 256;
                }
            }
        }
    }
    
    /* Final verification */
    int final_checksum = 0;
    for (int i = 0; i < N; i++) {
        final_checksum += static_array[i] + auto_array[i % N];
    }
    printf("Final checksum: %d\n", final_checksum);
    
    /* Cleanup */
    free(mat1);
    free(mat2);
    free(mat_result);
    
    return 0;
}
