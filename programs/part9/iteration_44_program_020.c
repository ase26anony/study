#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 1024
#define MAX_DIM 256

// Variant 1: SIMD target loop
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step) {
    volatile int v_start = start;  // Prevent constant folding
    volatile int v_end = end;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[v_start:v_end], b[v_start:v_end]) \
        map(from: c[v_start:v_end]) \
        private(v_start, v_end) \
        num_teams(4) thread_limit(128)
    for (int i = v_start; i < v_end; i += step) {
        c[i] = a[i] * 2 + b[i];
    }
}

// Variant 2: Parallel target loop without SIMD clause
void parallel_target_loop(float *x, float *y, float *z, int n, float scale) {
    static float static_scale = 1.0f;  // Mix static storage
    const float const_scale = scale;
    float local_scale = const_scale * static_scale;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[0:n], y[0:n]) \
        map(from: z[0:n]) \
        firstprivate(local_scale) \
        collapse(2) \
        num_teams(8)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {  // Fixed inner dimension
            int idx = i * 4 + j;
            if (idx < n) {
                z[idx] = x[idx] + local_scale * y[idx];
            }
        }
    }
    
    static_scale += 0.1f;  // Modify static variable
}

// Variant 3: Combined constructs with data region
void combined_constructs(double *mat1, double *mat2, double *result, 
                         int rows, int cols, int offset) {
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    
    // Create complex data environment with pointer arithmetic
    double *slice1 = mat1 + offset;
    double *slice2 = mat2 + offset;
    double *res_slice = result + offset;
    int slice_size = v_rows * v_cols - offset;
    
    #pragma omp target data map(to: slice1[0:slice_size], slice2[0:slice_size]) \
                            map(from: res_slice[0:slice_size])
    {
        #pragma omp target teams distribute parallel for simd \
            firstprivate(v_rows, v_cols, offset) \
            private(slice1, slice2, res_slice) \
            collapse(2)
        for (int i = 0; i < v_rows; i++) {
            for (int j = 0; j < v_cols; j++) {
                int idx = i * v_cols + j;
                if (idx >= offset && idx < slice_size + offset) {
                    res_slice[idx - offset] = slice1[idx - offset] * 
                                             slice2[idx - offset] / 
                                             (i + j + 1.0);
                }
            }
        }
    }
}

// Host-only parallel region for conditional execution
void host_only_parallel(int *arr, int n, int factor) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * factor + i;
    }
}

// Function that conditionally calls target or host version
void conditional_omp_region(int *data, int size, int use_target, int seed) {
    if (use_target) {
        int *temp1 = (int*)malloc(size * sizeof(int));
        int *temp2 = (int*)malloc(size * sizeof(int));
        
        // Initialize with pattern
        for (int i = 0; i < size; i++) {
            temp1[i] = (i + seed) % 100;
            temp2[i] = (i * seed) % 100;
        }
        
        // Call target version with runtime-dependent bounds
        int start = seed % (size/2);
        int end = size - (seed % (size/4));
        simd_target_loop(temp1, temp2, data, start, end, 1 + (seed % 3));
        
        free(temp1);
        free(temp2);
    } else {
        host_only_parallel(data, size, 2 + (seed % 5));
    }
}

int main(int argc, char *argv[]) {
    // Use command-line argument for seed
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    // Initialize arrays with different types and storage durations
    int *int_arr1 = (int*)malloc(SIZE * sizeof(int));
    int *int_arr2 = (int*)malloc(SIZE * sizeof(int));
    int *int_res = (int*)malloc(SIZE * sizeof(int));
    
    float *float_arr1 = (float*)malloc(SIZE * sizeof(float));
    float *float_arr2 = (float*)malloc(SIZE * sizeof(float));
    float *float_res = (float*)malloc(SIZE * sizeof(float));
    
    double *double_mat1 = (double*)malloc(MAX_DIM * MAX_DIM * sizeof(double));
    double *double_mat2 = (double*)malloc(MAX_DIM * MAX_DIM * sizeof(double));
    double *double_res = (double*)malloc(MAX_DIM * MAX_DIM * sizeof(double));
    
    // Initialize with random data
    for (int i = 0; i < SIZE; i++) {
        int_arr1[i] = rand() % 100;
        int_arr2[i] = rand() % 100;
        float_arr1[i] = (float)(rand() % 100) / 10.0f;
        float_arr2[i] = (float)(rand() % 100) / 10.0f;
    }
    
    for (int i = 0; i < MAX_DIM * MAX_DIM; i++) {
        double_mat1[i] = (double)(rand() % 100) / 5.0;
        double_mat2[i] = (double)(rand() % 100) / 5.0;
    }
    
    // Main loop with varying parameters
    int iterations = 5;
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d:\n", iter);
        
        // Vary parameters based on iteration and random seed
        int use_target = (iter + seed) % 2;  // Alternate between target and host
        int slice_start = (iter * 73) % (SIZE/2);
        int slice_end = SIZE - ((iter * 47) % (SIZE/4));
        float scale = 1.0f + (iter * 0.3f);
        int matrix_dim = 64 + (iter * 32);
        int offset = (iter * 11) % 100;
        
        // Call variant functions with different constructs
        printf("  Calling simd_target_loop...\n");
        simd_target_loop(int_arr1, int_arr2, int_res, slice_start, slice_end, 1);
        
        // Calculate checksum
        int int_checksum = 0;
        for (int i = slice_start; i < slice_end; i++) {
            int_checksum += int_res[i];
        }
        printf("  Integer checksum: %d\n", int_checksum);
        
        printf("  Calling parallel_target_loop...\n");
        parallel_target_loop(float_arr1, float_arr2, float_res, SIZE, scale);
        
        // Calculate checksum
        float float_checksum = 0.0f;
        for (int i = 0; i < SIZE; i++) {
            float_checksum += float_res[i];
        }
        printf("  Float checksum: %.2f\n", float_checksum);
        
        printf("  Calling combined_constructs...\n");
        combined_constructs(double_mat1, double_mat2, double_res, 
                           matrix_dim, matrix_dim, offset);
        
        // Calculate checksum
        double double_checksum = 0.0;
        int check_size = matrix_dim * matrix_dim - offset;
        if (check_size > 0) {
            for (int i = 0; i < check_size; i++) {
                double_checksum += double_res[i];
            }
        }
        printf("  Double checksum: %.2f\n", double_checksum);
        
        printf("  Calling conditional_omp_region...\n");
        conditional_omp_region(int_arr1, SIZE, use_target, seed + iter);
        
        printf("\n");
    }
    
    // Cleanup
    free(int_arr1);
    free(int_arr2);
    free(int_res);
    free(float_arr1);
    free(float_arr2);
    free(float_res);
    free(double_mat1);
    free(double_mat2);
    free(double_res);
    
    return 0;
}
