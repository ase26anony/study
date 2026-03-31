#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 1024
#define MAX_DIM 256

// Variant 1: SIMD target loop
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step) {
    volatile int vsize = end - start; // Prevent constant folding
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end-start], b[start:end-start]) \
        map(from: c[start:end-start]) \
        private(start) firstprivate(step) shared(vsize)
    for (int i = start; i < end; i += step) {
        int idx = i;
        if (idx < end && idx >= start) {
            c[idx] = a[idx] + b[idx] * (vsize % 16);
        }
    }
}

// Variant 2: Parallel target loop without SIMD clause
void parallel_target_loop(float *x, float *y, float scale, int low, int high) {
    static float static_buffer[MAX_DIM]; // Mixed storage duration
    const float const_factor = 3.14159f;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high-low], scale) \
        map(from: y[low:high-low]) \
        map(alloc: static_buffer[0:MAX_DIM/2])
    for (int i = low; i < high; i++) {
        float temp = x[i] * scale + const_factor;
        static_buffer[i % (MAX_DIM/2)] = temp; // Use static buffer
        y[i] = temp + static_buffer[(i + 1) % (MAX_DIM/2)];
    }
}

// Variant 3: Combined constructs with nested loops
void combined_constructs(double *mat1, double *mat2, double *result, 
                         int rows, int cols, int use_simd) {
    volatile int vrows = rows; // Volatile to prevent optimization
    volatile int vcols = cols;
    
    #pragma omp target data map(to: mat1[0:rows*cols], mat2[0:rows*cols]) \
                            map(from: result[0:rows*cols])
    {
        if (use_simd) {
            // Complex iteration space with collapse
            #pragma omp target teams distribute parallel for simd collapse(2) \
                firstprivate(vrows, vcols)
            for (int i = 0; i < vrows; i++) {
                for (int j = 0; j < vcols; j++) {
                    int idx = i * cols + j;
                    // Pointer arithmetic
                    double *ptr1 = mat1 + idx;
                    double *ptr2 = mat2 + idx;
                    result[idx] = *ptr1 * *ptr2 + (double)(i * j);
                }
            }
        } else {
            #pragma omp target teams distribute parallel for \
                firstprivate(vrows, vcols)
            for (int i = 0; i < vrows * vcols; i++) {
                result[i] = mat1[i] - mat2[i];
            }
        }
    }
}

// Variant 4: Host-only parallel region (for conditional execution)
void host_only_parallel(int *data, int n, int factor) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * factor + omp_get_thread_num();
    }
}

// Helper function with array sections
void process_section(int *arr, int start, int end, int stride) {
    volatile int local_start = start;
    volatile int local_end = end;
    
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[start:end-start:stride])
    for (int i = local_start; i < local_end; i += stride) {
        arr[i] = arr[i] * 2 + (i % 64);
    }
}

int main(int argc, char *argv[]) {
    // Use command-line argument for variability
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    // Declare arrays with different types and storage
    int *int_arr1 = (int*)malloc(SIZE * sizeof(int));
    int *int_arr2 = (int*)malloc(SIZE * sizeof(int));
    int *int_arr3 = (int*)malloc(SIZE * sizeof(int));
    float *float_arr1 = (float*)malloc(SIZE * sizeof(float));
    float *float_arr2 = (float*)malloc(SIZE * sizeof(float));
    double *double_mat1 = (double*)malloc(MAX_DIM * MAX_DIM * sizeof(double));
    double *double_mat2 = (double*)malloc(MAX_DIM * MAX_DIM * sizeof(double));
    double *double_result = (double*)malloc(MAX_DIM * MAX_DIM * sizeof(double));
    
    // Initialize with random data
    for (int i = 0; i < SIZE; i++) {
        int_arr1[i] = rand() % 100;
        int_arr2[i] = rand() % 100;
        int_arr3[i] = 0;
        float_arr1[i] = (float)rand() / RAND_MAX;
        float_arr2[i] = 0.0f;
    }
    
    for (int i = 0; i < MAX_DIM * MAX_DIM; i++) {
        double_mat1[i] = (double)rand() / RAND_MAX;
        double_mat2[i] = (double)rand() / RAND_MAX;
        double_result[i] = 0.0;
    }
    
    int checksum = 0;
    
    // Loop with varying parameters to expose different contexts
    for (int iter = 0; iter < 5; iter++) {
        // Runtime-dependent bounds
        int start = rand() % (SIZE/2);
        int end = start + 128 + (rand() % 128);
        int step = 1 + (rand() % 3);
        int use_simd_flag = rand() % 2;
        int rows = 32 + (rand() % 32);
        int cols = 32 + (rand() % 32);
        float scale = 1.0f + (float)(rand() % 100) / 100.0f;
        
        // Conditional execution to influence SIMT transformation
        if (rand() % 2) {
            printf("Iteration %d: Using target region\n", iter);
            simd_target_loop(int_arr1, int_arr2, int_arr3, start, end, step);
            
            // Verify results
            for (int i = start; i < end; i += step) {
                checksum += int_arr3[i];
            }
        } else {
            printf("Iteration %d: Using host-only parallel\n", iter);
            host_only_parallel(int_arr1, SIZE, iter + 1);
        }
        
        // Call different variants
        parallel_target_loop(float_arr1, float_arr2, scale, 
                            start % SIZE, (start + 64) % SIZE);
        
        combined_constructs(double_mat1, double_mat2, double_result,
                           rows, cols, use_simd_flag);
        
        // Process array section with stride
        int stride = 1 + (iter % 4);
        process_section(int_arr1, 0, SIZE, stride);
        
        // Update checksum from various computations
        for (int i = 0; i < SIZE; i += 16) {
            checksum += int_arr1[i] + (int)float_arr2[i % SIZE];
        }
        
        // Small matrix checksum
        for (int i = 0; i < rows * cols && i < 100; i++) {
            checksum += (int)double_result[i];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Seed used: %d\n", seed);
    
    // Cleanup
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
