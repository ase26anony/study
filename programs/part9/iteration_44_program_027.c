#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, 
                      int stride, volatile int n, int seed) {
    static int static_counter = 0;
    const int chunk_size = 64;
    int local_private = seed;
    
    /* Mixed storage duration and complex map clauses */
    #pragma omp target teams distribute parallel for simd \
        collapse(2) num_teams(4) thread_limit(128) \
        map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) \
        private(local_private) firstprivate(seed) \
        shared(static_counter)
    for (int i = start; i < end; i += stride) {
        for (int j = 0; j < chunk_size; j++) {
            /* Runtime-dependent indexing to prevent optimization */
            int idx = (i * 7919 + j * 1009 + seed) % n;
            if (idx < 0) idx = -idx;
            
            /* Complex computation with pointer arithmetic */
            int *ptr_a = &a[idx];
            int *ptr_b = &b[(idx + seed) % n];
            int *ptr_c = &c[idx];
            
            local_private = (*ptr_a * seed + *ptr_b) % 10007;
            *ptr_c = local_private + (j % 7);
            
            /* Access static variable through function call */
            #pragma omp atomic
            static_counter += (i % 3);
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float alpha, 
                         int low, int high, volatile int limit) {
    const float beta = 2.71828f;
    float firstprivate_var = alpha * beta;
    
    /* Array section with stride in map clause */
    #pragma omp target teams distribute parallel for \
        num_teams(8) thread_limit(64) \
        map(to: x[low:high:2], alpha) \
        map(tofrom: y[low:high:2]) \
        firstprivate(firstprivate_var, beta)
    for (int i = low; i < high; i += 2) {
        /* Non-linear access pattern */
        int idx = (i * 997 + (int)(firstprivate_var * 1000)) % limit;
        if (idx < 0) idx = -idx;
        
        /* Computation with conditional */
        if (idx % 3 == 0) {
            y[idx] = x[idx] * alpha + beta;
        } else if (idx % 3 == 1) {
            y[idx] = x[idx] / alpha - beta;
        } else {
            y[idx] = (x[idx] + firstprivate_var) * beta;
        }
        
        /* Use of math function to prevent optimization */
        y[idx] += (float)(idx % 5) * 0.1f;
    }
}

/* Variant 3: Combined constructs with nested data regions */
void combined_constructs(double *mat1, double *mat2, double *result,
                        int rows, int cols, int tile, int seed) {
    /* Separate data region */
    #pragma omp target data map(to: mat1[0:rows*cols]) \
                            map(to: mat2[0:rows*cols]) \
                            map(alloc: result[0:rows*cols])
    {
        int dynamic_chunk = (seed % 16) + 1;
        const int const_chunk = 32;
        
        /* Nested loop with collapse */
        #pragma omp target teams distribute parallel for simd \
            collapse(2) num_teams(rows/tile) thread_limit(256) \
            firstprivate(dynamic_chunk, const_chunk)
        for (int i = 0; i < rows; i += tile) {
            for (int j = 0; j < cols; j += tile) {
                /* Tile processing with runtime bounds */
                for (int ti = 0; ti < tile && (i + ti) < rows; ti++) {
                    for (int tj = 0; tj < tile && (j + tj) < cols; tj++) {
                        int idx = (i + ti) * cols + (j + tj);
                        int alt_idx = ((i + ti) * 7387 + (j + tj) * 1009) % (rows * cols);
                        
                        /* Complex computation with multiple array accesses */
                        double temp = mat1[idx] * mat2[alt_idx];
                        temp += (double)((ti * tj + seed) % 100) * 0.01;
                        
                        /* Conditional update */
                        if ((ti + tj) % dynamic_chunk == 0) {
                            result[idx] = temp * const_chunk;
                        } else {
                            result[idx] = temp / const_chunk;
                        }
                    }
                }
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int size, int factor) {
    #pragma omp parallel for simd schedule(dynamic, 16) \
        private(factor) shared(arr)
    for (int i = 0; i < size; i++) {
        arr[i] = (arr[i] * factor + i) % 100003;
    }
}

int main(int argc, char *argv[]) {
    /* Use volatile for critical values to prevent constant folding */
    volatile int use_target = 1;
    volatile int array_size = N;
    volatile int seed = 0;
    
    /* Initialize from command line or random */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    srand(seed);
    
    /* Allocate arrays with different types */
    int *int_arr1 = (int*)malloc(N * sizeof(int));
    int *int_arr2 = (int*)malloc(N * sizeof(int));
    int *int_arr3 = (int*)malloc(N * sizeof(int));
    
    float *float_arr1 = (float*)malloc(M * sizeof(float));
    float *float_arr2 = (float*)malloc(M * sizeof(float));
    
    double *double_mat1 = (double*)malloc(N * M * sizeof(double));
    double *double_mat2 = (double*)malloc(N * M * sizeof(double));
    double *double_result = (double*)malloc(N * M * sizeof(double));
    
    /* Initialize with random data */
    for (int i = 0; i < N; i++) {
        int_arr1[i] = rand() % 1000;
        int_arr2[i] = rand() % 1000;
        int_arr3[i] = 0;
    }
    
    for (int i = 0; i < M; i++) {
        float_arr1[i] = (float)(rand() % 1000) / 10.0f;
        float_arr2[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    for (int i = 0; i < N * M; i++) {
        double_mat1[i] = (double)(rand() % 1000) / 100.0;
        double_mat2[i] = (double)(rand() % 1000) / 100.0;
        double_result[i] = 0.0;
    }
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        /* Runtime-dependent condition to choose execution path */
        int condition = (rand() % 3);
        
        if (condition == 0) {
            /* Call target region with SIMD */
            int start = (iter * 17) % N;
            int end = start + 128 + (rand() % 128);
            int stride = 1 + (iter % 3);
            volatile int bound = end - start;
            
            simd_target_loop(int_arr1, int_arr2, int_arr3, 
                           start, end, stride, bound, seed + iter);
            
            /* Verify results */
            long checksum = 0;
            for (int i = start; i < end && i < N; i += stride) {
                checksum += int_arr3[i];
            }
            printf("Iter %d, SIMD target checksum: %ld\n", iter, checksum);
            
        } else if (condition == 1) {
            /* Call parallel target without SIMD */
            int low = (iter * 23) % M;
            int high = low + 64 + (rand() % 64);
            volatile int limit = M;
            float alpha = 1.0f + (iter % 5) * 0.25f;
            
            parallel_target_loop(float_arr1, float_arr2, alpha, 
                               low, high, limit);
            
            /* Verify results */
            double sum = 0.0;
            for (int i = low; i < high && i < M; i += 2) {
                sum += float_arr2[i];
            }
            printf("Iter %d, Parallel target sum: %.4f\n", iter, sum);
            
        } else {
            /* Call combined constructs */
            int tile_size = 8 + (iter % 8);
            int rows = N / 4;
            int cols = M / 2;
            
            combined_constructs(double_mat1, double_mat2, double_result,
                              rows, cols, tile_size, seed + iter * 7);
            
            /* Verify results */
            double total = 0.0;
            for (int i = 0; i < rows * cols; i += 67) {
                total += double_result[i];
            }
            printf("Iter %d, Combined constructs total: %.6f\n", iter, total);
        }
        
        /* Occasionally call host-only parallel region */
        if (iter % 2 == 0) {
            int factor = 1 + (rand() % 10);
            host_only_parallel(int_arr1, 256, factor);
            
            /* Quick verification */
            int quick_sum = 0;
            for (int i = 0; i < 256; i += 8) {
                quick_sum += int_arr1[i];
            }
            printf("  Host-only parallel quick sum: %d\n", quick_sum);
        }
    }
    
    /* Final verification */
    printf("\nFinal verification:\n");
    
    long final_int_sum = 0;
    for (int i = 0; i < N; i += 13) {
        final_int_sum += int_arr3[i];
    }
    printf("Integer array checksum: %ld\n", final_int_sum);
    
    double final_float_sum = 0.0;
    for (int i = 0; i < M; i += 7) {
        final_float_sum += float_arr2[i];
    }
    printf("Float array sum: %.4f\n", final_float_sum);
    
    double final_double_sum = 0.0;
    for (int i = 0; i < N * M; i += 131) {
        final_double_sum += double_result[i];
    }
    printf("Double matrix sum: %.6f\n", final_double_sum);
    
    /* Cleanup */
    free(int_arr1);
    free(int_arr2);
    free(int_arr3);
    free(float_arr1);
    free(float_arr2);
    free(double_mat1);
    free(double_mat2);
    free(double_result);
    
    return 0;
}
