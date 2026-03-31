#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

// Variant 1: SIMD target loop
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step) {
    volatile int v_start = start;  // Prevent constant folding
    volatile int v_end = end;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[v_start:v_end], b[v_start:v_end]) \
        map(from: c[v_start:v_end]) \
        private(v_start, v_end) \
        firstprivate(step)
    for (int i = v_start; i < v_end; i += step) {
        c[i] = a[i] + b[i] * step;
    }
}

// Variant 2: Parallel target loop without SIMD
void parallel_target_loop(float *x, float *y, float *z, 
                         int rows, int cols, float scale) {
    static const int chunk_size = 16;  // Mixed storage duration
    volatile int v_rows = rows;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[0:rows*cols], y[0:rows*cols]) \
        map(from: z[0:rows*cols]) \
        collapse(2) \
        firstprivate(scale, chunk_size)
    for (int i = 0; i < v_rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            z[idx] = x[idx] * scale + y[idx] / (chunk_size + 1);
        }
    }
}

// Variant 3: Combined constructs with data region
void combined_constructs(double *mat1, double *mat2, double *result,
                        int n, int m, int offset) {
    volatile int v_offset = offset;
    const int block = 64;
    
    #pragma omp target data map(to: mat1[0:n*m], mat2[v_offset:n*m]) \
                            map(from: result[0:n*m])
    {
        #pragma omp target teams distribute parallel for simd \
            firstprivate(v_offset, block) \
            private(n, m)
        for (int i = 0; i < n * m; i++) {
            // Complex addressing with pointer arithmetic
            double *ptr1 = mat1 + i;
            double *ptr2 = mat2 + (i + v_offset) % (n * m);
            result[i] = (*ptr1) * (*ptr2) + (i % block);
        }
    }
}

// Variant 4: Nested function with conditional SIMD
void nested_simt_test(int *arr, int size, int use_simd) {
    volatile int v_use_simd = use_simd;
    
    if (v_use_simd) {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: arr[0:size])
        for (int i = 0; i < size; i++) {
            arr[i] = arr[i] * 2 + i;
        }
    } else {
        #pragma omp parallel for
        for (int i = 0; i < size; i++) {
            arr[i] = arr[i] / 2 - i;
        }
    }
}

// Helper function with array sections
void process_section(int *data, int low, int high, int stride) {
    volatile int v_low = low;
    volatile int v_high = high;
    
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[v_low:v_high:v_stride]) \
        firstprivate(stride)
    for (int i = v_low; i < v_high; i += stride) {
        data[i] = data[i] * (i % 10) + stride;
    }
}

int main(int argc, char *argv[]) {
    // Use command-line argument for variability
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    // Allocate arrays with different types and storage
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(N * sizeof(int));
    
    float *x = (float *)malloc(N * M * sizeof(float));
    float *y = (float *)malloc(N * M * sizeof(float));
    float *z = (float *)malloc(N * M * sizeof(float));
    
    double *mat1 = (double *)malloc(N * M * sizeof(double));
    double *mat2 = (double *)malloc(N * M * sizeof(double));
    double *result = (double *)malloc(N * M * sizeof(double));
    
    int *test_arr = (int *)malloc(N * sizeof(int));
    
    // Initialize with random data
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = 0;
        test_arr[i] = rand() % 1000;
    }
    
    for (int i = 0; i < N * M; i++) {
        x[i] = (float)rand() / RAND_MAX;
        y[i] = (float)rand() / RAND_MAX;
        z[i] = 0.0f;
        mat1[i] = (double)rand() / RAND_MAX;
        mat2[i] = (double)rand() / RAND_MAX;
        result[i] = 0.0;
    }
    
    // Main loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        printf("Iteration %d:\n", iter);
        
        // Vary parameters using rand() to prevent optimization
        int start = rand() % (N/4);
        int end = N - (rand() % (N/4));
        int step = 1 + (rand() % 3);
        
        // Call variant functions with different patterns
        if (iter % 2 == 0) {
            simd_target_loop(a, b, c, start, end, step);
            
            // Compute checksum
            long sum = 0;
            for (int i = start; i < end; i += step) {
                sum += c[i];
            }
            printf("  SIMD target checksum: %ld\n", sum);
        }
        
        if (iter % 3 != 0) {
            int rows = 32 + (rand() % 64);
            int cols = 16 + (rand() % 32);
            float scale = 1.0f + (float)rand() / RAND_MAX;
            
            parallel_target_loop(x, y, z, rows, cols, scale);
            
            // Verify results
            float checksum = 0.0f;
            for (int i = 0; i < rows * cols; i++) {
                checksum += z[i];
            }
            printf("  Parallel target checksum: %.4f\n", checksum);
        }
        
        // Always test combined constructs
        int offset = rand() % 100;
        combined_constructs(mat1, mat2, result, 64, 32, offset);
        
        double total = 0.0;
        for (int i = 0; i < 64 * 32; i++) {
            total += result[i];
        }
        printf("  Combined constructs checksum: %.4f\n", total);
        
        // Test nested SIMT with condition
        int use_simd_flag = (rand() % 2);
        nested_simt_test(test_arr, N/2, use_simd_flag);
        
        // Test array sections
        int low = rand() % (N/2);
        int high = N - (rand() % (N/4));
        int stride = 1 + (rand() % 4);
        process_section(test_arr, low, high, stride);
        
        // Final verification
        int final_sum = 0;
        for (int i = 0; i < N; i++) {
            final_sum += test_arr[i];
        }
        printf("  Final array checksum: %d\n\n", final_sum);
    }
    
    // Cleanup
    free(a); free(b); free(c);
    free(x); free(y); free(z);
    free(mat1); free(mat2); free(result);
    free(test_arr);
    
    return 0;
}
