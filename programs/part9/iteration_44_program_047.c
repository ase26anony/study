#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

// Variant 1: SIMD target loop with complex data environment
void simd_target_loop(int *a, int *b, int *c, int start, int end, 
                      int stride, volatile int bound) {
    static int static_counter = 0;
    const int chunk_size = 64;
    int private_var = start;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) private(private_var) \
        firstprivate(static_counter) shared(chunk_size) num_teams(bound/256) \
        thread_limit(256)
    for (int i = start; i < end; i += stride) {
        for (int j = 0; j < chunk_size; j++) {
            int idx = i + j % stride;
            if (idx < end) {
                private_var = a[idx] * (j + 1);
                c[idx] = private_var + b[idx] + static_counter;
            }
        }
    }
    static_counter++;
}

// Variant 2: Parallel target loop without SIMD clause
void parallel_target_loop(float *x, float *y, float *z, int low, int high,
                          volatile int teams, volatile int threads) {
    float local_accum = 0.0f;
    const float scale = 2.5f;
    
    #pragma omp target data map(to: x[low:high], y[low:high]) \
                            map(from: z[low:high])
    {
        #pragma omp target teams distribute parallel for \
                num_teams(teams) thread_limit(threads) \
                reduction(+:local_accum) collapse(1)
        for (int i = low; i < high; i++) {
            float temp = x[i] * scale + y[i];
            z[i] = temp * (i % 32 + 1);
            local_accum += z[i];
        }
    }
    
    // Use result to prevent elimination
    if (local_accum > 1000000.0f) {
        printf("Large accumulation: %f\n", local_accum);
    }
}

// Variant 3: Combined constructs with pointer arithmetic
void combined_constructs(double *mat1, double *mat2, double *result,
                         int rows, int cols, int ld, volatile int flag) {
    double *row_ptr = mat1;
    double *col_ptr = mat2;
    const double alpha = 1.5;
    const double beta = 0.5;
    
    #pragma omp target data map(to: mat1[0:rows*cols], mat2[0:rows*cols]) \
                            map(from: result[0:rows*cols])
    {
        if (flag & 0x1) {
            #pragma omp target teams distribute parallel for simd \
                    collapse(2) num_teams((rows+15)/16) thread_limit(128)
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    int idx = i * ld + j;
                    double val1 = mat1[idx];
                    double val2 = mat2[j * ld + i];  // Transposed access
                    result[idx] = alpha * val1 + beta * val2 + (i * j * 0.01);
                }
            }
        } else {
            #pragma omp target teams distribute parallel for \
                    num_teams(rows/32) thread_limit(64)
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    int idx = i * ld + j;
                    result[idx] = mat1[idx] * mat2[idx];
                }
            }
        }
    }
}

// Host-only parallel region for conditional execution
void host_only_parallel(int *arr, int size, int factor) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum) simd
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * factor + i;
        sum += arr[i];
    }
    printf("Host parallel sum: %d\n", sum);
}

int main(int argc, char *argv[]) {
    // Use command-line argument for variability
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    // Volatile variables to prevent constant folding
    volatile int v_size = N + (rand() % 256);
    volatile int v_offset = rand() % 128;
    volatile int v_stride = 1 + (rand() % 4);
    volatile int v_teams = 4 + (rand() % 8);
    volatile int v_threads = 64 + (rand() % 192);
    
    // Allocate arrays with different storage durations
    static int static_array[N];
    int auto_array[N];
    const int const_size = M;
    float float_array[M];
    double double_matrix[N * M];
    double double_matrix2[N * M];
    double result_matrix[N * M];
    
    // Initialize arrays with pattern
    for (int i = 0; i < N; i++) {
        static_array[i] = i * 2 + 1;
        auto_array[i] = i * 3 - 1;
        if (i < M) {
            float_array[i] = i * 1.5f;
        }
    }
    
    for (int i = 0; i < N * M; i++) {
        double_matrix[i] = (i % 100) * 0.1;
        double_matrix2[i] = (i % 50) * 0.2;
    }
    
    printf("Starting OpenMP SIMT transformation test (seed: %d)\n", seed);
    
    // Main test loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        printf("\nIteration %d:\n", iter);
        
        // Vary parameters each iteration
        int start = (iter * 64) % N;
        int end = start + 256 + (rand() % 128);
        int stride = 1 + (iter % 3);
        
        // Conditional execution based on random value
        int choice = rand() % 4;
        
        if (choice == 0 || choice == 2) {
            // Call target region functions
            int temp_result[N];
            memcpy(temp_result, auto_array, sizeof(auto_array));
            
            simd_target_loop(static_array, auto_array, temp_result,
                           start, end, stride, v_size);
            
            // Compute checksum
            long checksum = 0;
            for (int i = start; i < end && i < N; i += stride) {
                checksum += temp_result[i];
            }
            printf("SIMD target checksum: %ld\n", checksum);
            
            if (choice == 2) {
                parallel_target_loop(float_array, float_array, float_array,
                                   0, const_size, v_teams, v_threads);
            }
        } else {
            // Call host-only function
            host_only_parallel(auto_array, v_size % N, iter + 2);
        }
        
        // Always call combined constructs with varying flags
        int flag = rand() % 3;
        combined_constructs(double_matrix, double_matrix2, result_matrix,
                          N / (iter + 1), M / (iter + 1), M, flag);
        
        // Verify result with simple check
        double verify_sum = 0.0;
        for (int i = 0; i < 100; i++) {
            int idx = (i * 17) % (N * M);
            verify_sum += result_matrix[idx];
        }
        printf("Matrix verify sum: %f\n", verify_sum);
        
        // Modify input arrays for next iteration
        for (int i = 0; i < N; i++) {
            auto_array[i] += iter;
        }
        for (int i = 0; i < M; i++) {
            float_array[i] *= 1.1f;
        }
    }
    
    printf("\nTest completed.\n");
    return 0;
}
