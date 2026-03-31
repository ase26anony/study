#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

// Variant 1: SIMD target loop
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step, 
                      volatile int bound_modifier) {
    const int local_n = N + bound_modifier;
    static int static_buffer[N];
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end], b[start:end]) map(from: c[start:end]) \
        map(tofrom: static_buffer[0:local_n]) \
        private(start) firstprivate(end) shared(step)
    for (int i = start; i < end; i += step) {
        int idx = i % local_n;
        c[i] = a[i] * 2 + b[i] + static_buffer[idx];
        static_buffer[idx] = (c[i] + i) % 256;
    }
}

// Variant 2: Parallel target loop without SIMD clause
void parallel_target_loop(float *x, float *y, float *z, int rows, int cols,
                         volatile int use_simd) {
    float *temp = (float*)malloc(rows * cols * sizeof(float));
    const int total = rows * cols;
    
    #pragma omp target data map(to: x[0:total], y[0:total]) \
                            map(from: z[0:total]) \
                            map(alloc: temp[0:total])
    {
        #pragma omp target teams distribute parallel for collapse(2) \
            if(target: use_simd > 0)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                temp[idx] = x[idx] * 1.5f;
                z[idx] = temp[idx] + y[idx] * 0.5f;
            }
        }
    }
    
    free(temp);
}

// Variant 3: Combined constructs with complex data environment
void combined_constructs(double *mat1, double *mat2, double *result,
                        int dim1, int dim2, int dim3, volatile int seed) {
    static double shared_static[1024];
    double local_stack[256];
    
    // Initialize local stack with pattern
    for (int i = 0; i < 256; i++) {
        local_stack[i] = (i + seed) * 0.1;
    }
    
    #pragma omp target data map(to: mat1[0:dim1*dim2], mat2[0:dim2*dim3]) \
                            map(from: result[0:dim1*dim3]) \
                            map(tofrom: shared_static[0:512])
    {
        #pragma omp target teams distribute parallel for simd collapse(2) \
            firstprivate(dim2, dim3) private(local_stack)
        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim3; j++) {
                double sum = 0.0;
                int result_idx = i * dim3 + j;
                
                // Matrix multiplication kernel
                for (int k = 0; k < dim2; k++) {
                    int idx1 = i * dim2 + k;
                    int idx2 = k * dim3 + j;
                    sum += mat1[idx1] * mat2[idx2];
                }
                
                // Use static and stack variables
                int static_idx = (i * dim3 + j) % 512;
                int stack_idx = (i + j) % 256;
                result[result_idx] = sum + shared_static[static_idx] + local_stack[stack_idx];
                shared_static[static_idx] = result[result_idx] * 0.01;
            }
        }
    }
}

// Host-only parallel region for conditional execution
void host_only_parallel(int *arr, int size, volatile int factor) {
    #pragma omp parallel for simd
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * factor + i;
    }
}

int main(int argc, char *argv[]) {
    // Use command-line argument for seed
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    // Declare arrays with different storage durations
    static int static_array[N];
    int auto_array[N];
    float float_array[M];
    double double_array[N];
    
    // Initialize arrays with random data
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = rand() % 100;
        double_array[i] = (double)(rand() % 1000) / 10.0;
    }
    for (int i = 0; i < M; i++) {
        float_array[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    // Result arrays
    int result_int[N];
    float result_float[M];
    double result_double[N];
    
    // Volatile variables to prevent constant folding
    volatile int v_bound = rand() % 100;
    volatile int v_use_simd = rand() % 2;
    volatile int v_seed = rand() % 1000;
    
    // Main loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        printf("Iteration %d:\n", iter);
        
        // Vary parameters each iteration
        int start = iter * 100;
        int end = start + 200 + (iter * 50);
        int step = 1 + (iter % 3);
        
        // Conditional execution based on random value
        if (rand() % 2) {
            // Call SIMD target variant
            memset(result_int, 0, N * sizeof(int));
            simd_target_loop(static_array, auto_array, result_int, 
                           start % N, end % N, step, v_bound + iter);
            
            // Compute checksum
            long long checksum = 0;
            for (int i = 0; i < N; i++) {
                checksum += result_int[i];
            }
            printf("  SIMD Target Checksum: %lld\n", checksum);
        } else {
            // Call host-only variant
            host_only_parallel(auto_array, N, v_bound);
            
            // Compute checksum
            long long checksum = 0;
            for (int i = 0; i < N; i++) {
                checksum += auto_array[i];
            }
            printf("  Host-only Checksum: %lld\n", checksum);
        }
        
        // Call parallel target variant
        memset(result_float, 0, M * sizeof(float));
        int rows = 16 + iter * 4;
        int cols = 32;
        parallel_target_loop(float_array, float_array, result_float, 
                           rows, cols, v_use_simd + iter);
        
        // Compute checksum
        float fchecksum = 0.0f;
        for (int i = 0; i < M; i++) {
            fchecksum += result_float[i];
        }
        printf("  Parallel Target Checksum: %.2f\n", fchecksum);
        
        // Call combined constructs variant
        memset(result_double, 0, N * sizeof(double));
        int dim1 = 8 + iter;
        int dim2 = 16;
        int dim3 = 8;
        combined_constructs(double_array, double_array, result_double,
                          dim1, dim2, dim3, v_seed + iter);
        
        // Compute checksum
        double dchecksum = 0.0;
        for (int i = 0; i < N; i++) {
            dchecksum += result_double[i];
        }
        printf("  Combined Constructs Checksum: %.2f\n", dchecksum);
        
        printf("\n");
    }
    
    return 0;
}
