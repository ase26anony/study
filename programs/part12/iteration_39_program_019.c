#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

int use_gpu_offload = 0;

#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_function(float a, float b, int i) {
    return a * b + i * 0.5f;
}

void test_target_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(4) safelen(8) \
        aligned(a, b, c: 16) \
        private(i) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] + simd_function(a[i], b[i], i);
    }
}

void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) \
        aligned(a, b, c: 32) \
        linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] - i;
    }
}

void test_nested_simd(double *a, double *b, double *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for collapse(2) simd \
            simdlen(2) \
            private(i, j)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] / (b[idx] + 1.0) + i * j;
            }
        }
    }
}

void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        #pragma omp simd simdlen(4)
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx < n * 4) {
                c[idx] = a[idx] * 2.0f - b[idx];
            }
        }
    }
}

void test_simd_with_dynamic_data(float **a, float **b, float **c, int rows, int cols) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: a[0:rows][0:cols], b[0:rows][0:cols]) \
        map(from: c[0:rows][0:cols]) \
        collapse(2) simdlen(2)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            c[i][j] = a[i][j] + b[i][j] * (i + j);
        }
    }
}

int main(int argc, char **argv) {
    // Parse command line argument for GPU offloading
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    // Allocate and initialize test data
    float *a_f = (float*)malloc(N * sizeof(float));
    float *b_f = (float*)malloc(N * sizeof(float));
    float *c_f = (float*)malloc(N * sizeof(float));
    
    int *a_i = (int*)malloc(N * sizeof(int));
    int *b_i = (int*)malloc(N * sizeof(int));
    int *c_i = (int*)malloc(N * sizeof(int));
    
    double *a_d = (double*)malloc(N * M * sizeof(double));
    double *b_d = (double*)malloc(N * M * sizeof(double));
    double *c_d = (double*)malloc(N * M * sizeof(double));
    
    // Initialize with patterned data
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 1.5f;
        b_f[i] = N - i * 0.5f;
        a_i[i] = i;
        b_i[i] = N - i;
    }
    
    for (int i = 0; i < N * M; i++) {
        a_d[i] = i * 0.25;
        b_d[i] = i * 0.75;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    // Test 1: Target SIMD with conditional offloading
    printf("\nTest 1: Target SIMD with conditional offloading\n");
    test_target_simd(a_f, b_f, c_f, N);
    
    // Compute checksum
    float sum_f = 0.0f;
    for (int i = 0; i < N; i++) {
        sum_f += c_f[i];
    }
    printf("Checksum test 1: %f\n", sum_f);
    
    // Test 2: Parallel for SIMD (host-only)
    printf("\nTest 2: Parallel for SIMD (host-only)\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    int sum_i = 0;
    for (int i = 0; i < N; i++) {
        sum_i += c_i[i];
    }
    printf("Checksum test 2: %d\n", sum_i);
    
    // Test 3: Nested SIMD
    printf("\nTest 3: Nested SIMD with collapse\n");
    test_nested_simd(a_d, b_d, c_d, N, M);
    
    double sum_d = 0.0;
    for (int i = 0; i < N * M; i++) {
        sum_d += c_d[i];
    }
    printf("Checksum test 3: %lf\n", sum_d);
    
    // Test 4: Mixed directives
    printf("\nTest 4: Mixed directives (target + inner SIMD)\n");
    test_mixed_directives(a_f, b_f, c_f, N/4);
    
    sum_f = 0.0f;
    for (int i = 0; i < N; i++) {
        sum_f += c_f[i];
    }
    printf("Checksum test 4: %f\n", sum_f);
    
    // Test 5: Dynamic 2D data
    printf("\nTest 5: SIMD with dynamic 2D data\n");
    int rows = 16, cols = 16;
    float **a_2d = (float**)malloc(rows * sizeof(float*));
    float **b_2d = (float**)malloc(rows * sizeof(float*));
    float **c_2d = (float**)malloc(rows * sizeof(float*));
    
    for (int i = 0; i < rows; i++) {
        a_2d[i] = (float*)malloc(cols * sizeof(float));
        b_2d[i] = (float*)malloc(cols * sizeof(float));
        c_2d[i] = (float*)malloc(cols * sizeof(float));
        for (int j = 0; j < cols; j++) {
            a_2d[i][j] = i * cols + j;
            b_2d[i][j] = (rows * cols) - (i * cols + j);
        }
    }
    
    test_simd_with_dynamic_data(a_2d, b_2d, c_2d, rows, cols);
    
    sum_f = 0.0f;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            sum_f += c_2d[i][j];
        }
    }
    printf("Checksum test 5: %f\n", sum_f);
    
    // Cleanup
    for (int i = 0; i < rows; i++) {
        free(a_2d[i]);
        free(b_2d[i]);
        free(c_2d[i]);
    }
    free(a_2d);
    free(b_2d);
    free(c_2d);
    
    free(a_f);
    free(b_f);
    free(c_f);
    free(a_i);
    free(b_i);
    free(c_i);
    free(a_d);
    free(b_d);
    free(c_d);
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}
