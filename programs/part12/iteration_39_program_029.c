#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

int use_gpu_offload = 0;

// Function with declare simd pragma
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_add(float a, float b, int i) {
    return a + b + (i * 0.1f);
}

// Test 1: Target SIMD with conditional offloading
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(64) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    } else {
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c:32) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

// Test 2: Parallel for SIMD with various clauses
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c:64) \
        schedule(static, 16) private(i) collapse(1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
    }
}

// Test 3: Nested SIMD with collapse
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4) safelen(8) private(i,j)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = simd_add(a[idx], b[idx], idx);
            }
        }
    }
}

// Test 4: Teams distribute parallel for simd with reduction
void test_teams_distribute_simd(float *a, float *b, float *c, int n) {
    float sum = 0.0f;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(tofrom: c[0:n]) \
        reduction(+:sum) simdlen(4) num_teams(8) thread_limit(128)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] - b[i];
        sum += c[i];
    }
    
    printf("Reduction sum: %f\n", sum);
}

// Test 5: Mixed directives - SIMD inside parallel region
void test_mixed_directives(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            #pragma omp simd simdlen(2) safelen(4)
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                c[idx] = a[idx] / (b[idx] + 1.0);
            }
        }
    }
}

// Test 6: Dynamic pointer-based access with SIMD
void test_pointer_simd(float **a, float **b, float **c, int n, int m) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n][0:m], b[0:n][0:m]) map(from: c[0:n][0:m]) \
        simdlen(4) is_device_ptr(a, b, c)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            c[i][j] = a[i][j] * b[i][j];
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
    
    // Allocate and initialize arrays
    float *a_f = (float*)malloc(N * sizeof(float));
    float *b_f = (float*)malloc(N * sizeof(float));
    float *c_f = (float*)malloc(N * sizeof(float));
    
    int *a_i = (int*)malloc(N * sizeof(int));
    int *b_i = (int*)malloc(N * sizeof(int));
    int *c_i = (int*)malloc(N * sizeof(int));
    
    double *a_d = (double*)malloc(N * 4 * sizeof(double));
    double *b_d = (double*)malloc(N * 4 * sizeof(double));
    double *c_d = (double*)malloc(N * 4 * sizeof(double));
    
    // Initialize with patterned data
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 1.0f;
        b_f[i] = (N - i) * 1.0f;
        a_i[i] = i;
        b_i[i] = N - i;
    }
    
    for (int i = 0; i < N * 4; i++) {
        a_d[i] = i * 0.5;
        b_d[i] = (N * 4 - i) * 0.5;
    }
    
    // Run test functions
    printf("Test 1: Target SIMD\n");
    test_target_simd(a_f, b_f, c_f, N);
    
    // Compute checksum to prevent dead code elimination
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N; i++) {
        checksum1 += c_f[i];
    }
    printf("Checksum 1: %f\n", checksum1);
    
    printf("\nTest 2: Parallel for SIMD\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    int checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N; i++) {
        checksum2 += c_i[i];
    }
    printf("Checksum 2: %d\n", checksum2);
    
    printf("\nTest 3: Nested SIMD\n");
    test_nested_simd(a_f, b_f, c_f, N/4, 4);
    
    float checksum3 = 0.0f;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < N; i++) {
        checksum3 += c_f[i];
    }
    printf("Checksum 3: %f\n", checksum3);
    
    printf("\nTest 4: Teams distribute with reduction\n");
    test_teams_distribute_simd(a_f, b_f, c_f, N);
    
    printf("\nTest 5: Mixed directives\n");
    test_mixed_directives(a_d, b_d, c_d, N);
    
    double checksum5 = 0.0;
    #pragma omp simd reduction(+:checksum5)
    for (int i = 0; i < N * 4; i++) {
        checksum5 += c_d[i];
    }
    printf("Checksum 5: %lf\n", checksum5);
    
    // Test 6: Pointer-based access
    printf("\nTest 6: Pointer-based SIMD\n");
    float **a_ptr = (float**)malloc(M * sizeof(float*));
    float **b_ptr = (float**)malloc(M * sizeof(float*));
    float **c_ptr = (float**)malloc(M * sizeof(float*));
    
    for (int i = 0; i < M; i++) {
        a_ptr[i] = (float*)malloc(M * sizeof(float));
        b_ptr[i] = (float*)malloc(M * sizeof(float));
        c_ptr[i] = (float*)malloc(M * sizeof(float));
        
        for (int j = 0; j < M; j++) {
            a_ptr[i][j] = i * M + j;
            b_ptr[i][j] = (M * M) - (i * M + j);
        }
    }
    
    if (use_gpu_offload) {
        // Note: This would require proper device allocation in real code
        // For testing the transformation, we'll just call it
        // test_pointer_simd(a_ptr, b_ptr, c_ptr, M, M);
    }
    
    // Cleanup
    free(a_f); free(b_f); free(c_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    
    for (int i = 0; i < M; i++) {
        free(a_ptr[i]); free(b_ptr[i]); free(c_ptr[i]);
    }
    free(a_ptr); free(b_ptr); free(c_ptr);
    
    return 0;
}
