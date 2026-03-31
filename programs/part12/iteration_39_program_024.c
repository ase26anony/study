#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64

// Global flag to control GPU offloading
static int use_gpu_offload = 0;

// Function declared with SIMD attribute
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_add(float a, float b, int i) {
    return a + b + (i * 0.001f);
}

// Test 1: Target SIMD with conditional offloading
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) num_teams(4) thread_limit(128)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    } else {
        // Host fallback
        #pragma omp simd simdlen(4) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

// Test 2: Parallel for SIMD with various clauses
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(8) safelen(16) \
        private(a, b, c) aligned(a, b, c: 64) linear(i:1) collapse(1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
    }
}

// Test 3: Nested SIMD with collapse clause
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = simd_add(a[idx], b[idx], idx);
            }
        }
    }
}

// Test 4: Teams distribute with SIMD and reduction
float test_reduction_simd(float *data, int n) {
    float sum = 0.0f;
    
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: data[0:n]) map(tofrom: sum) \
            reduction(+:sum) simdlen(4) num_teams(8)
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    } else {
        #pragma omp parallel for simd reduction(+:sum) simdlen(8)
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    }
    
    return sum;
}

// Test 5: Mixed directives - SIMD inside parallel region
void test_mixed_directives(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            #pragma omp simd simdlen(4)
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < n * 4) {
                    c[idx] = a[idx] - b[idx];
                }
            }
        }
    }
}

// Test 6: Dynamic arrays with pointer arithmetic
void test_dynamic_arrays(int **matrix_a, int **matrix_b, int **matrix_c, int rows, int cols) {
    #pragma omp target teams distribute parallel for simd \
        map(to: matrix_a[0:rows][0:cols], matrix_b[0:rows][0:cols]) \
        map(from: matrix_c[0:rows][0:cols]) \
        simdlen(4) collapse(2) if(use_gpu_offload)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix_c[i][j] = matrix_a[i][j] + matrix_b[i][j];
        }
    }
}

int main(int argc, char *argv[]) {
    // Parse command line argument
    if (argc > 1 && strcmp(argv[1], "--use-gpu") == 0) {
        use_gpu_offload = 1;
        printf("GPU offloading enabled\n");
    } else {
        printf("GPU offloading disabled (host-only)\n");
    }
    
    // Allocate and initialize arrays
    float *a_f = (float*)aligned_alloc(64, N * sizeof(float));
    float *b_f = (float*)aligned_alloc(64, N * sizeof(float));
    float *c_f = (float*)aligned_alloc(64, N * sizeof(float));
    
    int *a_i = (int*)aligned_alloc(64, N * sizeof(int));
    int *b_i = (int*)aligned_alloc(64, N * sizeof(int));
    int *c_i = (int*)aligned_alloc(64, N * sizeof(int));
    
    double *a_d = (double*)aligned_alloc(64, N * sizeof(double));
    double *b_d = (double*)aligned_alloc(64, N * sizeof(double));
    double *c_d = (double*)aligned_alloc(64, N * sizeof(double));
    
    // Initialize with patterned data
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 0.5f;
        b_f[i] = (N - i) * 0.5f;
        a_i[i] = i;
        b_i[i] = N - i;
        a_d[i] = i * 0.25;
        b_d[i] = (N - i) * 0.25;
    }
    
    // Test 1: Target SIMD
    printf("Test 1: Target SIMD\n");
    test_target_simd(a_f, b_f, c_f, N);
    
    // Compute checksum
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N; i++) {
        checksum1 += c_f[i];
    }
    printf("  Checksum: %f\n", checksum1);
    
    // Test 2: Parallel for SIMD
    printf("Test 2: Parallel for SIMD\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    int checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N; i++) {
        checksum2 += c_i[i];
    }
    printf("  Checksum: %d\n", checksum2);
    
    // Test 3: Nested SIMD
    printf("Test 3: Nested SIMD\n");
    test_nested_simd(a_f, b_f, c_f, N/4, 4);
    
    float checksum3 = 0.0f;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < N; i++) {
        checksum3 += c_f[i];
    }
    printf("  Checksum: %f\n", checksum3);
    
    // Test 4: Reduction SIMD
    printf("Test 4: Reduction SIMD\n");
    float sum = test_reduction_simd(a_f, N);
    printf("  Sum: %f\n", sum);
    
    // Test 5: Mixed directives
    printf("Test 5: Mixed directives\n");
    test_mixed_directives(a_d, b_d, c_d, N/4);
    
    double checksum5 = 0.0;
    #pragma omp simd reduction(+:checksum5)
    for (int i = 0; i < N; i++) {
        checksum5 += c_d[i];
    }
    printf("  Checksum: %lf\n", checksum5);
    
    // Test 6: Dynamic arrays (2D)
    printf("Test 6: Dynamic arrays\n");
    int rows = 32, cols = 32;
    int **matrix_a = (int**)malloc(rows * sizeof(int*));
    int **matrix_b = (int**)malloc(rows * sizeof(int*));
    int **matrix_c = (int**)malloc(rows * sizeof(int*));
    
    for (int i = 0; i < rows; i++) {
        matrix_a[i] = (int*)aligned_alloc(64, cols * sizeof(int));
        matrix_b[i] = (int*)aligned_alloc(64, cols * sizeof(int));
        matrix_c[i] = (int*)aligned_alloc(64, cols * sizeof(int));
        
        for (int j = 0; j < cols; j++) {
            matrix_a[i][j] = i * cols + j;
            matrix_b[i][j] = (rows - i) * (cols - j);
        }
    }
    
    test_dynamic_arrays(matrix_a, matrix_b, matrix_c, rows, cols);
    
    int checksum6 = 0;
    for (int i = 0; i < rows; i++) {
        #pragma omp simd reduction(+:checksum6)
        for (int j = 0; j < cols; j++) {
            checksum6 += matrix_c[i][j];
        }
    }
    printf("  Checksum: %d\n", checksum6);
    
    // Cleanup
    free(a_f); free(b_f); free(c_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    
    for (int i = 0; i < rows; i++) {
        free(matrix_a[i]);
        free(matrix_b[i]);
        free(matrix_c[i]);
    }
    free(matrix_a);
    free(matrix_b);
    free(matrix_c);
    
    return 0;
}
