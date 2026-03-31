#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 1024
#define MAX_DIM 256

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step, 
                      volatile int n, int offset) {
    static int static_counter = 0;
    const int chunk_size = 16;
    int private_var = offset;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) map(to: a[start:end], b[start:end]) map(from: c[start:end]) \
        private(private_var) firstprivate(static_counter) shared(chunk_size)
    for (int i = 0; i < n; i += step) {
        for (int j = 0; j < chunk_size; j++) {
            int idx = i + j + offset;
            if (idx < end) {
                private_var = a[idx] * static_counter;
                c[idx] = private_var + b[idx] * (i % 8);
            }
        }
    }
    
    static_counter++;
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, 
                          int low, int high, int stride,
                          volatile int limit) {
    float local_accum = 0.0f;
    const float scale = 2.5f;
    
    #pragma omp target data map(to: x[low:high:stride]) \
                            map(tofrom: y[low:high:stride]) \
                            map(from: z[low:high:stride])
    {
        #pragma omp target teams distribute parallel for \
            private(local_accum) firstprivate(scale) reduction(+:local_accum)
        for (int i = low; i < high; i += stride) {
            local_accum = x[i] * scale;
            y[i] = y[i] + local_accum;
            z[i] = y[i] * 0.5f;
        }
    }
}

/* Variant 3: Combined constructs with pointer arithmetic */
void combined_constructs(double *arr1, double *arr2, double *arr3,
                         int *mask, volatile int dim1, volatile int dim2) {
    double *ptr1 = arr1;
    double *ptr2 = arr2 + 8;
    const double coeff = 3.14159;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: ptr1[0:dim1], ptr2[-8:dim2]) \
        map(tofrom: arr3[0:dim1*dim2]) \
        map(to: mask[0:dim1])
    for (int i = 0; i < dim1; i++) {
        if (mask[i] > 0) {
            for (int j = 0; j < dim2; j++) {
                int idx = i * dim2 + j;
                arr3[idx] = ptr1[i] * coeff + ptr2[j] * (i % 4);
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *data, int n, int factor) {
    int local_factor = factor;
    
    #pragma omp parallel for simd private(local_factor)
    for (int i = 0; i < n; i++) {
        local_factor = factor + (i % 16);
        data[i] = data[i] * local_factor;
    }
}

/* Function that selects between target and host execution */
void conditional_execution(int *a, int *b, int *c, int size, 
                           int use_target, volatile int bound) {
    if (use_target) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:size], b[0:size]) map(from: c[0:size])
        for (int i = 0; i < bound; i++) {
            c[i] = a[i] + b[i] * (i % 32);
        }
    } else {
        host_only_parallel(c, size, bound);
    }
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Declare arrays with different storage durations */
    static int static_array[SIZE];
    int auto_array[SIZE];
    const int const_size = SIZE;
    volatile int vol_bound = SIZE / 2 + rand() % 256;
    
    float float_arr1[SIZE], float_arr2[SIZE], float_arr3[SIZE];
    double double_arr1[MAX_DIM * MAX_DIM];
    double double_arr2[MAX_DIM * MAX_DIM];
    double double_arr3[MAX_DIM * MAX_DIM];
    int mask[MAX_DIM];
    
    /* Initialize with random data */
    for (int i = 0; i < SIZE; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = rand() % 100;
        float_arr1[i] = (float)(rand() % 100) / 10.0f;
        float_arr2[i] = (float)(rand() % 100) / 10.0f;
    }
    
    for (int i = 0; i < MAX_DIM; i++) {
        mask[i] = rand() % 2;
        for (int j = 0; j < MAX_DIM; j++) {
            int idx = i * MAX_DIM + j;
            double_arr1[idx] = (double)(rand() % 100) / 3.0;
            double_arr2[idx] = (double)(rand() % 100) / 3.0;
        }
    }
    
    int result_array[SIZE] = {0};
    int checksum = 0;
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < 5; iter++) {
        int use_target = (iter % 2 == 0) ? 1 : 0;
        int offset = rand() % 64;
        int stride = 1 + (iter % 4);
        int dim1 = 64 + (iter * 32);
        int dim2 = 32 + (iter * 16);
        
        /* Update volatile bounds to prevent constant folding */
        vol_bound = SIZE / 4 + rand() % (SIZE / 2);
        
        /* Call variant functions with different patterns */
        if (iter % 3 == 0) {
            simd_target_loop(static_array, auto_array, result_array,
                             offset, SIZE, stride, vol_bound, iter);
        } else if (iter % 3 == 1) {
            parallel_target_loop(float_arr1, float_arr2, float_arr3,
                                 0, vol_bound, stride, vol_bound);
        } else {
            combined_constructs(double_arr1, double_arr2, double_arr3,
                                mask, dim1, dim2);
        }
        
        /* Conditional execution based on runtime value */
        conditional_execution(static_array, auto_array, result_array,
                             SIZE, use_target, vol_bound);
        
        /* Compute checksum to prevent dead code elimination */
        for (int i = 0; i < SIZE; i++) {
            checksum += result_array[i] + (int)float_arr3[i];
        }
        
        printf("Iteration %d: checksum = %d, vol_bound = %d\n", 
               iter, checksum, vol_bound);
    }
    
    /* Additional test with nested function calls */
    {
        int temp[SIZE];
        memcpy(temp, static_array, SIZE * sizeof(int));
        
        #pragma omp target data map(to: temp[0:SIZE])
        {
            #pragma omp target teams distribute parallel for simd \
                collapse(2) num_teams(4) thread_limit(128)
            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < SIZE/16; j++) {
                    int idx = i * (SIZE/16) + j;
                    temp[idx] = temp[idx] * (i + j) + checksum;
                }
            }
        }
        
        /* Final verification */
        int final_sum = 0;
        #pragma omp parallel for reduction(+:final_sum)
        for (int i = 0; i < SIZE; i++) {
            final_sum += temp[i];
        }
        printf("Final sum: %d\n", final_sum);
    }
    
    return 0;
}
