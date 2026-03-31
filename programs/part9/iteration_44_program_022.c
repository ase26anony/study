#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 1024
#define MAX_DIM 256

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step, 
                      volatile int n, int use_simd) {
    static int static_counter = 0;
    const int chunk_size = 16;
    int local_n = n;
    
    /* Mixed storage duration variables */
    int auto_var = start;
    static int static_var = 0;
    const int const_var = 100;
    
    /* Create non-trivial data environment */
    int *ptr_arith = a + start;
    int firstprivate_val = auto_var * 2;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end-start], b[start:end-start]) \
        map(from: c[start:end-start]) \
        firstprivate(firstprivate_val) \
        private(auto_var) \
        shared(static_var) \
        if(use_simd > 0)
    for (int i = start; i < end; i += step) {
        auto_var = i % chunk_size;
        int idx = (i * 13) % (end - start);  /* Non-linear access pattern */
        c[idx] = a[idx] + b[idx] + firstprivate_val + auto_var + static_var;
        static_var = (static_var + 1) % 100;
    }
    
    static_counter++;
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, 
                         int rows, int cols, volatile int limit) {
    int total = rows * cols;
    float scale = 2.5f;
    
    /* Nested loop structure that might get collapsed */
    #pragma omp target teams distribute parallel for \
        map(to: x[0:total], y[0:total]) \
        map(from: z[0:total]) \
        collapse(2) \
        num_teams(4) \
        thread_limit(128)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            if (idx < limit) {
                z[idx] = x[idx] * scale + y[idx] / (j + 1);
            }
        }
    }
}

/* Variant 3: Combined constructs with data region */
void combined_constructs(double *mat1, double *mat2, double *result,
                        int dim, volatile int iter) {
    double temp[MAX_DIM * MAX_DIM];
    
    /* Target data region followed by compute region */
    #pragma omp target data map(to: mat1[0:dim*dim], mat2[0:dim*dim]) \
                            map(alloc: temp[0:dim*dim])
    {
        /* First kernel: matrix transpose */
        #pragma omp target teams distribute parallel for simd \
            map(always, to: mat1[0:dim*dim]) \
            map(from: temp[0:dim*dim]) \
            num_teams(dim/16) \
            thread_limit(64)
        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                temp[j * dim + i] = mat1[i * dim + j];
            }
        }
        
        /* Second kernel: matrix multiplication */
        #pragma omp target teams distribute parallel for simd \
            map(to: temp[0:dim*dim], mat2[0:dim*dim]) \
            map(from: result[0:dim*dim]) \
            collapse(2)
        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                double sum = 0.0;
                for (int k = 0; k < dim; k++) {
                    sum += temp[i * dim + k] * mat2[k * dim + j];
                }
                result[i * dim + j] = sum / (iter + 1);
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int n, int factor) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * factor + omp_get_thread_num();
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for variability */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Volatile variables to prevent constant folding */
    volatile int v_size = SIZE + (rand() % 128);
    volatile int v_iter = 5 + (rand() % 3);
    volatile int use_target = (rand() % 2);
    
    /* Arrays with different types */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    int *arr3 = (int*)malloc(SIZE * sizeof(int));
    
    float *farr1 = (float*)malloc(SIZE * sizeof(float));
    float *farr2 = (float*)malloc(SIZE * sizeof(float));
    float *farr3 = (float*)malloc(SIZE * sizeof(float));
    
    double *dmat1 = (double*)malloc(MAX_DIM * MAX_DIM * sizeof(double));
    double *dmat2 = (double*)malloc(MAX_DIM * MAX_DIM * sizeof(double));
    double *dresult = (double*)malloc(MAX_DIM * MAX_DIM * sizeof(double));
    
    /* Initialize with random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = 0;
        
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = (float)rand() / RAND_MAX;
        farr3[i] = 0.0f;
    }
    
    int dim = 64 + (rand() % (MAX_DIM - 64));
    for (int i = 0; i < dim * dim; i++) {
        dmat1[i] = (double)rand() / RAND_MAX;
        dmat2[i] = (double)rand() / RAND_MAX;
        dresult[i] = 0.0;
    }
    
    printf("Starting with seed: %d, use_target: %d\n", seed, use_target);
    
    /* Main loop calling variant functions */
    for (int iter = 0; iter < v_iter; iter++) {
        int start = (iter * 100) % SIZE;
        int end = start + 200 + (rand() % 100);
        int step = 1 + (iter % 3);
        
        /* Conditional execution path */
        if (use_target || (rand() % 3 == 0)) {
            /* Call target region functions */
            simd_target_loop(arr1, arr2, arr3, start, end, step, 
                           v_size, use_target);
            
            int rows = 32 + (iter * 8);
            int cols = 32 + (iter * 4);
            parallel_target_loop(farr1, farr2, farr3, rows, cols, v_size);
            
            if (iter % 2 == 0) {
                combined_constructs(dmat1, dmat2, dresult, dim, iter);
            }
        } else {
            /* Host-only path */
            host_only_parallel(arr1, SIZE, iter + 1);
        }
        
        /* Compute checksums to prevent dead code elimination */
        int int_sum = 0;
        float float_sum = 0.0f;
        double double_sum = 0.0;
        
        #pragma omp parallel for reduction(+:int_sum, float_sum, double_sum)
        for (int i = 0; i < SIZE; i++) {
            int_sum += arr3[i];
            float_sum += farr3[i];
        }
        
        for (int i = 0; i < dim * dim; i++) {
            double_sum += dresult[i];
        }
        
        printf("Iter %d: Checksums - int: %d, float: %.3f, double: %.3f\n",
               iter, int_sum, float_sum, double_sum);
        
        /* Modify volatile variable for next iteration */
        v_size += (rand() % 10) - 5;
    }
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr1);
    free(farr2);
    free(farr3);
    free(dmat1);
    free(dmat2);
    free(dresult);
    
    return 0;
}
