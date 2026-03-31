#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64

int use_gpu_offload = 0;

// Function with declare simd pragma
#pragma omp declare simd uniform(a, b) linear(i:1) notinbranch
float simd_func(float a, float b, int i) {
    return a * b + i * 0.5f;
}

// Test 1: Target SIMD with conditional offloading
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) num_teams(4) thread_limit(128)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

// Test 2: Parallel for SIMD with various clauses
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(8) safelen(16) \
        aligned(a, b, c: 64) schedule(static, 32) private(a, b, c)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

// Test 3: Nested SIMD with collapse
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4) linear(i, j:1)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = simd_func(a[idx], b[idx], idx);
            }
        }
    }
}

// Test 4: Reduction with SIMD
float test_simd_reduction(float *a, float *b, int n) {
    float sum = 0.0f;
    
    #pragma omp parallel for simd reduction(+:sum) simdlen(4) safelen(8)
    for (int i = 0; i < n; i++) {
        sum += a[i] * b[i];
    }
    
    return sum;
}

// Test 5: Teams distribute parallel for SIMD with dynamic data
void test_teams_distribute_simd(float **a, float **b, float **c, int rows, int cols) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:rows][0:cols], b[0:rows][0:cols]) \
        map(from: c[0:rows][0:cols]) \
        collapse(2) simdlen(4) num_teams(rows/16) thread_limit(64)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            c[i][j] = a[i][j] + b[i][j] * 3.0f;
        }
    }
}

// Test 6: Mixed directives - SIMD inside parallel region
void test_mixed_directives(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i += 64) {
            #pragma omp simd simdlen(8) aligned(a, b, c: 64)
            for (int j = i; j < i + 64 && j < n; j++) {
                c[j] = a[j] * 2.5 + b[j] * 1.5;
            }
        }
    }
}

int main(int argc, char **argv) {
    // Parse command line argument
    if (argc > 1 && strcmp(argv[1], "--use-gpu") == 0) {
        use_gpu_offload = 1;
        printf("GPU offloading enabled\n");
    } else {
        printf("Host-only execution\n");
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
        a_f[i] = i * 1.0f;
        b_f[i] = (N - i) * 1.0f;
        c_f[i] = 0.0f;
        
        a_i[i] = i;
        b_i[i] = N - i;
        c_i[i] = 0;
        
        a_d[i] = i * 0.5;
        b_d[i] = (N - i) * 0.5;
        c_d[i] = 0.0;
    }
    
    // Test 1: Target SIMD with conditional execution
    printf("Test 1: Target SIMD\n");
    test_target_simd(a_f, b_f, c_f, N);
    
    // Compute checksum
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N; i++) {
        checksum1 += c_f[i];
    }
    printf("  Checksum1: %f\n", checksum1);
    
    // Test 2: Parallel for SIMD
    printf("Test 2: Parallel for SIMD\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    int checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N; i++) {
        checksum2 += c_i[i];
    }
    printf("  Checksum2: %d\n", checksum2);
    
    // Test 3: Nested SIMD
    printf("Test 3: Nested SIMD\n");
    test_nested_simd(a_f, b_f, c_f, N/2, 2);
    
    float checksum3 = 0.0f;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < N; i++) {
        checksum3 += c_f[i];
    }
    printf("  Checksum3: %f\n", checksum3);
    
    // Test 4: SIMD reduction
    printf("Test 4: SIMD reduction\n");
    float reduction_sum = test_simd_reduction(a_f, b_f, N);
    printf("  Reduction sum: %f\n", reduction_sum);
    
    // Test 5: Dynamic 2D arrays for teams distribute
    printf("Test 5: Teams distribute with 2D arrays\n");
    int rows = 32, cols = 32;
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
            c_2d[i][j] = 0.0f;
        }
    }
    
    if (use_gpu_offload) {
        test_teams_distribute_simd(a_2d, b_2d, c_2d, rows, cols);
    }
    
    // Test 6: Mixed directives
    printf("Test 6: Mixed directives\n");
    test_mixed_directives(a_d, b_d, c_d, N);
    
    double checksum6 = 0.0;
    #pragma omp simd reduction(+:checksum6)
    for (int i = 0; i < N; i++) {
        checksum6 += c_d[i];
    }
    printf("  Checksum6: %lf\n", checksum6);
    
    // Validation: Compare results if both paths were tested
    if (use_gpu_offload) {
        // Re-run test 1 on host for comparison
        float *c_f_host = (float*)aligned_alloc(64, N * sizeof(float));
        int save_flag = use_gpu_offload;
        use_gpu_offload = 0;
        test_target_simd(a_f, b_f, c_f_host, N);
        use_gpu_offload = save_flag;
        
        // Compare results
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (fabs(c_f[i] - c_f_host[i]) > 1e-6) {
                errors++;
                if (errors < 5) {
                    printf("Mismatch at index %d: GPU=%f, Host=%f\n", 
                           i, c_f[i], c_f_host[i]);
                }
            }
        }
        printf("Validation: %d errors found\n", errors);
        
        free(c_f_host);
    }
    
    // Cleanup
    free(a_f); free(b_f); free(c_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    
    for (int i = 0; i < rows; i++) {
        free(a_2d[i]); free(b_2d[i]); free(c_2d[i]);
    }
    free(a_2d); free(b_2d); free(c_2d);
    
    return 0;
}
