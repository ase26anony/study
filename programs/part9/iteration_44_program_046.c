#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define MAX_DIM 256

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int stride, 
                      volatile int n, int offset) {
    static int static_counter = 0;
    const int chunk_size = 32;
    int local_private = offset;
    
    /* Mixed storage duration and complex mapping */
    #pragma omp target teams distribute parallel for simd \
        collapse(2) \
        map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) \
        private(local_private) \
        firstprivate(static_counter) \
        shared(chunk_size) \
        num_teams(n/64) thread_limit(128)
    for (int i = start; i < end; i += stride) {
        for (int j = 0; j < chunk_size; j++) {
            /* Complex index calculation to prevent optimization */
            int idx = (i * 31 + j * 17 + offset) % (end - start);
            if (idx < 0) idx = -idx;
            
            /* Runtime-dependent computation */
            c[idx] = a[idx] * b[idx] + local_private + 
                    (static_counter % 2) * (j & 1);
            local_private = (local_private * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    static_counter++;
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, 
                         int low, int high, int step,
                         volatile int teams, volatile int threads) {
    float local_accum = 0.0f;
    const float scale = 2.5f;
    
    /* Pointer arithmetic in map clauses */
    float *x_slice = x + low;
    float *y_slice = y + low;
    float *z_slice = z + low;
    int slice_len = high - low;
    
    #pragma omp target teams distribute parallel for \
        map(to: x_slice[0:slice_len:step], y_slice[0:slice_len:step]) \
        map(tofrom: z_slice[0:slice_len:step]) \
        reduction(+:local_accum) \
        num_teams(teams) num_threads(threads) \
        if(slice_len > 100)
    for (int i = 0; i < slice_len; i += step) {
        /* Non-linear access pattern */
        int idx = (i * 7 + 13) % slice_len;
        z_slice[idx] = x_slice[idx] * scale + y_slice[idx] / scale;
        
        /* Conditional execution within loop */
        if (idx % 3 == 0) {
            z_slice[idx] += 1.0f;
            local_accum += z_slice[idx];
        } else if (idx % 7 == 0) {
            z_slice[idx] -= 0.5f;
            local_accum -= z_slice[idx];
        }
    }
    
    /* Use result to prevent dead code elimination */
    if (local_accum > 1000.0f) {
        printf("Accumulator: %f\n", local_accum);
    }
}

/* Variant 3: Combined constructs with nested data regions */
void combined_constructs(double *mat1, double *mat2, double *result,
                        int rows, int cols, int tile,
                        volatile int use_simd) {
    /* Static array with mixed qualifiers */
    static volatile int tile_sizes[4] = {16, 32, 64, 128};
    const int max_tile = tile_sizes[tile % 4];
    
    /* Target data region enclosing compute region */
    #pragma omp target data map(to: mat1[0:rows*cols]) \
                            map(to: mat2[0:rows*cols]) \
                            map(from: result[0:rows*cols])
    {
        /* Conditional SIMD transformation trigger */
        if (use_simd) {
            #pragma omp target teams distribute parallel for simd \
                collapse(2) \
                private(tile) \
                firstprivate(max_tile) \
                num_teams((rows+max_tile-1)/max_tile) \
                thread_limit(256)
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    int idx = i * cols + j;
                    /* Complex computation with runtime dependency */
                    result[idx] = mat1[idx] * mat2[idx] + 
                                 (i % max_tile) * (j % max_tile) +
                                 (tile & 1) * 0.5;
                }
            }
        } else {
            #pragma omp target teams distribute parallel for \
                collapse(2) \
                num_teams(rows/32) num_threads(128)
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    int idx = i * cols + j;
                    result[idx] = mat1[idx] + mat2[idx] - 
                                 (i * j) * 0.01;
                }
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *data, int size, int factor) {
    int local_sum = 0;
    
    #pragma omp parallel for reduction(+:local_sum) \
        if(size > 500) \
        schedule(dynamic, 16)
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * factor + (i % 8);
        local_sum += data[i];
    }
    
    printf("Host sum: %d\n", local_sum);
}

/* Function that selects between target and host execution */
void conditional_execution(int *arr1, int *arr2, int size, 
                          int choice, volatile int bound) {
    if (choice % 3 == 0) {
        /* Call target region with SIMD */
        simd_target_loop(arr1, arr2, arr1, 0, size, 2, bound, choice);
    } else if (choice % 3 == 1) {
        /* Call host-only parallel */
        host_only_parallel(arr1, size, choice);
    } else {
        /* Mixed execution */
        host_only_parallel(arr1, size/2, choice);
        simd_target_loop(arr1 + size/2, arr2 + size/2, arr1 + size/2,
                       0, size/2, 1, bound/2, choice);
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for variability */
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Volatile variables to prevent constant folding */
    volatile int v_size = ARRAY_SIZE + (rand() % 100);
    volatile int v_teams = 4 + (rand() % 8);
    volatile int v_threads = 32 + (rand() % 96);
    volatile int v_use_simd = rand() % 2;
    
    /* Arrays with different types and storage */
    int *int_arr1 = (int*)malloc(v_size * sizeof(int));
    int *int_arr2 = (int*)malloc(v_size * sizeof(int));
    float *float_arr1 = (float*)malloc(v_size * sizeof(float));
    float *float_arr2 = (float*)malloc(v_size * sizeof(float));
    double *double_mat1 = (double*)malloc(MAX_DIM * MAX_DIM * sizeof(double));
    double *double_mat2 = (double*)malloc(MAX_DIM * MAX_DIM * sizeof(double));
    double *double_result = (double*)malloc(MAX_DIM * MAX_DIM * sizeof(double));
    
    /* Initialize with random/sequential data */
    for (int i = 0; i < v_size; i++) {
        int_arr1[i] = i + 1;
        int_arr2[i] = (i * 3) % 100;
        float_arr1[i] = (float)i * 0.5f;
        float_arr2[i] = (float)(i % 50) * 1.5f;
    }
    
    for (int i = 0; i < MAX_DIM * MAX_DIM; i++) {
        double_mat1[i] = (double)(i % 100) * 0.01;
        double_mat2[i] = (double)((i + 37) % 100) * 0.02;
    }
    
    printf("Starting OpenMP offloading tests (seed: %d)\n", seed);
    
    /* Loop with varying parameters to expose multiple contexts */
    for (int iter = 0; iter < 5; iter++) {
        printf("\n=== Iteration %d ===\n", iter);
        
        /* Vary parameters each iteration */
        int start = (iter * 17) % (v_size / 2);
        int end = v_size - (iter * 13) % (v_size / 4);
        int stride = 1 + (iter % 3);
        
        /* Call variant functions with different patterns */
        if (iter % 2 == 0) {
            simd_target_loop(int_arr1, int_arr2, int_arr1, 
                           start, end, stride, v_teams, iter);
        } else {
            parallel_target_loop(float_arr1, float_arr2, float_arr1,
                               start, end, stride, v_teams, v_threads);
        }
        
        /* Every third iteration, use combined constructs */
        if (iter % 3 == 0) {
            int dim = 64 + (iter * 8) % 128;
            combined_constructs(double_mat1, double_mat2, double_result,
                              dim, dim, iter, v_use_simd);
        }
        
        /* Conditional execution based on random state */
        conditional_execution(int_arr2, int_arr1, v_size / 2, 
                            rand() % 10, v_size);
        
        /* Compute checksums to prevent elimination */
        int int_sum = 0;
        float float_sum = 0.0f;
        double double_sum = 0.0;
        
        #pragma omp parallel for reduction(+:int_sum) if(v_size > 100)
        for (int i = 0; i < v_size; i++) {
            int_sum += int_arr1[i] + int_arr2[i];
        }
        
        #pragma omp parallel for reduction(+:float_sum) schedule(static, 32)
        for (int i = 0; i < v_size; i++) {
            float_sum += float_arr1[i] + float_arr2[i];
        }
        
        int check_dim = 64;
        #pragma omp parallel for collapse(2) reduction(+:double_sum)
        for (int i = 0; i < check_dim; i++) {
            for (int j = 0; j < check_dim; j++) {
                double_sum += double_result[i * check_dim + j];
            }
        }
        
        printf("Checksums - Int: %d, Float: %.2f, Double: %.4f\n",
               int_sum, float_sum, double_sum);
        
        /* Modify volatile variables for next iteration */
        v_teams += 1 + (iter % 3);
        v_threads = 32 + ((v_threads * 5 + 1) % 96);
        v_use_simd ^= 1;
    }
    
    /* Cleanup */
    free(int_arr1);
    free(int_arr2);
    free(float_arr1);
    free(float_arr2);
    free(double_mat1);
    free(double_mat2);
    free(double_result);
    
    printf("\nTest completed.\n");
    return 0;
}
