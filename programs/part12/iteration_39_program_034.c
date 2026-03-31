#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64

/* Global flag to control GPU offloading */
static int use_gpu_offload = 0;

/* Function declared with SIMD attribute */
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_add(float a, float b, int i) {
    return a + b + (i * 0.001f);
}

/* Test 1: Target teams distribute parallel for simd with conditional execution */
void test_target_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        num_teams(4) thread_limit(128) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(8) safelen(16) aligned(a, b, c: 32) \
        private(n) reduction(+:n)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] + simd_add(a[i], b[i], i);
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(4) safelen(8) collapse(2) \
        aligned(a, b, c: 16) linear(i:1) \
        schedule(static, 16)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c[idx] = a[idx] * 2 + b[idx] / 3;
        }
    }
}

/* Test 3: Nested SIMD with collapse */
void test_nested_simd(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) \
            simdlen(2) safelen(4) \
            private(n) lastprivate(i, j)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] * b[idx] - (i + j) * 0.5;
            }
        }
    }
}

/* Test 4: SIMD with reduction and linear clause */
void test_simd_reduction(float *arr, int n, float *result) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) linear(i:1) simdlen(16)
    for (int i = 0; i < n; i++) {
        sum += arr[i] * (i % 8);
    }
    
    *result = sum;
}

/* Test 5: Mixed directives - SIMD inside parallel region */
void test_mixed_directives(int *a, int *b, int *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            #pragma omp simd simdlen(4) aligned(a, b, c: 32)
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = (a[idx] + b[idx]) * (i - j);
            }
        }
    }
}

/* Helper function to compute checksum */
float compute_checksum(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Parse command line argument for GPU offloading */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    /* Allocate and initialize arrays */
    float *a_f = (float *)aligned_alloc(32, N * M * sizeof(float));
    float *b_f = (float *)aligned_alloc(32, N * M * sizeof(float));
    float *c_f = (float *)aligned_alloc(32, N * M * sizeof(float));
    
    int *a_i = (int *)aligned_alloc(16, N * M * sizeof(int));
    int *b_i = (int *)aligned_alloc(16, N * M * sizeof(int));
    int *c_i = (int *)aligned_alloc(16, N * M * sizeof(int));
    
    double *a_d = (double *)aligned_alloc(64, N * M * sizeof(double));
    double *b_d = (double *)aligned_alloc(64, N * M * sizeof(double));
    double *c_d = (double *)aligned_alloc(64, N * M * sizeof(double));
    
    /* Initialize with patterned data */
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < N * M; i++) {
        a_f[i] = i * 0.1f;
        b_f[i] = (N * M - i) * 0.2f;
        c_f[i] = 0.0f;
        
        a_i[i] = i % 256;
        b_i[i] = (i * 3) % 256;
        c_i[i] = 0;
        
        a_d[i] = i * 0.01;
        b_d[i] = (N * M - i) * 0.02;
        c_d[i] = 0.0;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("\nTest 1: Target teams distribute parallel for simd\n");
    test_target_simd(a_f, b_f, c_f, N * M);
    float checksum1 = compute_checksum(c_f, N * M);
    printf("Checksum 1: %f\n", checksum1);
    
    /* Test 2: Parallel for simd */
    printf("\nTest 2: Parallel for simd\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    float sum_i = 0.0f;
    #pragma omp simd reduction(+:sum_i) simdlen(4)
    for (int i = 0; i < N * M; i++) {
        sum_i += c_i[i];
    }
    printf("Checksum 2: %f\n", sum_i);
    
    /* Test 3: Nested SIMD */
    printf("\nTest 3: Nested SIMD with collapse\n");
    test_nested_simd(a_d, b_d, c_d, N);
    double sum_d = 0.0;
    #pragma omp simd reduction(+:sum_d)
    for (int i = 0; i < N * M; i++) {
        sum_d += c_d[i];
    }
    printf("Checksum 3: %f\n", (float)sum_d);
    
    /* Test 4: SIMD reduction */
    printf("\nTest 4: SIMD with reduction\n");
    float reduction_result = 0.0f;
    test_simd_reduction(a_f, N * M, &reduction_result);
    printf("Reduction result: %f\n", reduction_result);
    
    /* Test 5: Mixed directives */
    printf("\nTest 5: Mixed directives\n");
    test_mixed_directives(a_i, b_i, c_i, N);
    float sum_mixed = 0.0f;
    #pragma omp simd reduction(+:sum_mixed) simdlen(8)
    for (int i = 0; i < N * M; i++) {
        sum_mixed += c_i[i];
    }
    printf("Checksum 5: %f\n", sum_mixed);
    
    /* Validation: Compare with sequential computation if GPU was used */
    if (use_gpu_offload) {
        printf("\nValidating GPU results...\n");
        float *c_ref = (float *)malloc(N * M * sizeof(float));
        
        /* Sequential reference computation */
        for (int i = 0; i < N * M; i++) {
            c_ref[i] = a_f[i] + b_f[i] + simd_add(a_f[i], b_f[i], i);
        }
        
        float ref_checksum = 0.0f;
        for (int i = 0; i < N * M; i++) {
            ref_checksum += c_ref[i];
        }
        
        printf("GPU checksum: %f, Reference checksum: %f\n", 
               checksum1, ref_checksum);
        printf("Difference: %f\n", checksum1 - ref_checksum);
        
        free(c_ref);
    }
    
    /* Cleanup */
    free(a_f); free(b_f); free(c_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    
    printf("\nAll tests completed.\n");
    return 0;
}
