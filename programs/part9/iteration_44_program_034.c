#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int stride, 
                      volatile int n, volatile int m) {
    static int static_counter = 0;
    const int chunk_size = 32;
    int private_var = start;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) private(private_var) \
        firstprivate(chunk_size) shared(static_counter)
    for (int i = 0; i < n; i += chunk_size) {
        for (int j = 0; j < m; j++) {
            int idx = (i * m + j) % (end - start);
            if (idx >= 0 && idx < (end - start)) {
                c[start + idx] = a[start + idx] + b[start + idx] + 
                                private_var + static_counter;
                private_var = (private_var + 1) % 100;
            }
        }
    }
    static_counter++;
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, int low, int high, 
                         volatile int limit, int use_simd) {
    float local_accum = 0.0f;
    const float scale = 2.5f;
    
    /* Create conditional execution path that might influence SIMT decision */
    if (use_simd) {
        #pragma omp target teams distribute parallel for \
            map(to: x[low:high], y[low:high]) map(from: z[low:high]) \
            reduction(+:local_accum) firstprivate(scale)
        for (int i = low; i < high; i++) {
            z[i] = x[i] * scale + y[i];
            local_accum += z[i];
            /* Complex indexing to prevent optimization */
            if (i % 3 == 0) {
                z[i] += (float)(i % 7);
            }
        }
    } else {
        #pragma omp target teams distribute parallel for \
            map(to: x[low:high], y[low:high]) map(from: z[low:high])
        for (int i = low; i < high; i++) {
            z[i] = x[i] + y[i];
        }
    }
}

/* Variant 3: Combined constructs with nested data regions */
void combined_constructs(double *mat1, double *mat2, double *result, 
                        int rows, int cols, int offset) {
    volatile int dynamic_rows = rows;
    volatile int dynamic_cols = cols;
    int *temp_indices = (int*)malloc(rows * sizeof(int));
    
    /* Initialize indices with non-linear pattern */
    for (int i = 0; i < rows; i++) {
        temp_indices[i] = (i * 7 + 3) % cols;
    }
    
    #pragma omp target data map(to: mat1[0:rows*cols], mat2[0:rows*cols], \
                                temp_indices[0:rows]) \
                            map(from: result[0:rows*cols])
    {
        #pragma omp target teams distribute parallel for simd \
            collapse(2) firstprivate(offset)
        for (int i = 0; i < dynamic_rows; i++) {
            for (int j = 0; j < dynamic_cols; j++) {
                int idx = i * cols + j;
                int mod_idx = (idx + offset) % (rows * cols);
                result[idx] = mat1[idx] * mat2[mod_idx] + 
                             (double)temp_indices[i % rows];
                
                /* Conditional operation to create branching */
                if (j % 4 == 0) {
                    result[idx] += 1.0;
                } else if (j % 4 == 1) {
                    result[idx] -= 0.5;
                }
            }
        }
    }
    
    free(temp_indices);
}

/* Variant 4: Host-only parallel region for comparison */
void host_only_parallel(int *arr1, int *arr2, int size) {
    #pragma omp parallel for simd
    for (int i = 0; i < size; i++) {
        arr1[i] = arr1[i] * 2 + arr2[i];
    }
}

/* Helper function to verify results */
double verify_checksum(int *arr, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

float verify_float_checksum(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for random seed */
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Declare arrays with different storage durations */
    static int static_array[N];
    int auto_array[N];
    const int const_size = N;
    volatile int vol_size = M;
    
    float float_array_x[N];
    float float_array_y[N];
    float float_array_z[N];
    
    double matrix1[M * M];
    double matrix2[M * M];
    double result_matrix[M * M];
    
    /* Initialize arrays with random/sequential data */
    for (int i = 0; i < N; i++) {
        static_array[i] = i;
        auto_array[i] = rand() % 100;
        float_array_x[i] = (float)(rand() % 100) / 10.0f;
        float_array_y[i] = (float)(rand() % 100) / 10.0f;
    }
    
    for (int i = 0; i < M * M; i++) {
        matrix1[i] = (double)(rand() % 100) / 5.0;
        matrix2[i] = (double)(rand() % 100) / 5.0;
    }
    
    printf("Starting OpenMP SIMT transformation test with seed %d\n", seed);
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        printf("\n=== Iteration %d ===\n", iter);
        
        /* Vary parameters to create different transformation contexts */
        int start = iter * 100;
        int end = start + 200 + (rand() % 100);
        int stride = 1 + (iter % 3);
        int use_simd_flag = (iter % 2);
        int offset = iter * 50;
        
        volatile int dynamic_n = N - iter * 20;
        volatile int dynamic_m = M - iter * 10;
        
        /* Call variant functions with different patterns */
        if (iter % 3 == 0) {
            printf("Calling simd_target_loop...\n");
            simd_target_loop(static_array, auto_array, auto_array, 
                           start, end, stride, dynamic_n, dynamic_m);
            double checksum = verify_checksum(auto_array, N);
            printf("Checksum: %.2f\n", checksum);
        }
        
        if (iter % 2 == 0 || iter == 1) {
            printf("Calling parallel_target_loop...\n");
            parallel_target_loop(float_array_x, float_array_y, float_array_z,
                               0, N, dynamic_n, use_simd_flag);
            float fchecksum = verify_float_checksum(float_array_z, N);
            printf("Float checksum: %.2f\n", fchecksum);
        }
        
        if (iter % 4 != 3) {
            printf("Calling combined_constructs...\n");
            int matrix_size = M - iter * 5;
            combined_constructs(matrix1, matrix2, result_matrix,
                              matrix_size, matrix_size, offset);
            
            /* Simple verification */
            double sum = 0.0;
            for (int i = 0; i < matrix_size * matrix_size; i += matrix_size + 1) {
                sum += result_matrix[i];
            }
            printf("Matrix diagonal sum: %.2f\n", sum);
        }
        
        /* Occasionally call host-only version */
        if (rand() % 3 == 0) {
            printf("Calling host_only_parallel...\n");
            host_only_parallel(auto_array, static_array, 
                             N - iter * 50);
            double checksum = verify_checksum(auto_array, N);
            printf("Host-only checksum: %.2f\n", checksum);
        }
        
        /* Modify some data for next iteration */
        for (int i = 0; i < 50; i++) {
            int idx = rand() % N;
            auto_array[idx] = rand() % 1000;
            float_array_x[idx] = (float)(rand() % 200) / 10.0f;
        }
    }
    
    printf("\nTest completed.\n");
    return 0;
}
