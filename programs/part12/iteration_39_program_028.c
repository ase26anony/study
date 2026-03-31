/* test_simt_coverage.c - Comprehensive OpenMP SIMD/SIMT test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32
#define CHUNK_SIZE 64

/* Global flag to control GPU offloading */
static int use_gpu_offload = 0;

/* Function with declare simd for vectorization */
#pragma omp declare simd uniform(b) linear(i:1) notinbranch
float simd_mul(float a, float b, int i) {
    return a * b + i * 0.1f;
}

/* Test 1: Target teams distribute parallel for simd with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(128) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(8) safelen(16) collapse(1) \
            private(n) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_mul(a[i], b[i], i);
        }
    } else {
        /* Host fallback with same SIMD pattern */
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 16) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(4) safelen(8) collapse(2) \
        schedule(static, CHUNK_SIZE) \
        private(a, b, c) aligned(a, b, c: 64) \
        reduction(+:n)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            if (idx < n) {
                c[idx] = a[idx] * 2 + b[idx] / 3;
                n += idx;  /* Reduction variable */
            }
        }
    }
}

/* Test 3: Nested SIMD with collapse and linear clauses */
void test_nested_simd(double *a, double *b, double *c, int n) {
    #pragma omp parallel num_threads(4)
    {
        #pragma omp for simd collapse(2) \
            simdlen(2) safelen(4) \
            linear(i, j:1) aligned(a, b, c: 128)
        for (int i = 0; i < n/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                c[idx] = a[idx] * b[idx] - (double)(i + j);
            }
        }
        
        /* Additional SIMD region in same parallel section */
        #pragma omp simd simdlen(8)
        for (int i = 0; i < n; i += 2) {
            c[i] += c[i+1];
        }
    }
}

/* Test 4: Mixed directives - simd inside for */
void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp parallel for
    for (int block = 0; block < n; block += CHUNK_SIZE) {
        int end = block + CHUNK_SIZE;
        if (end > n) end = n;
        
        #pragma omp simd simdlen(4) safelen(8) \
            aligned(a, b, c: 16) linear(k:1)
        for (int k = block; k < end; k++) {
            c[k] = a[k] - b[k] * 0.5f;
        }
    }
}

/* Test 5: SIMD with if clause and dynamic data */
void test_simd_with_if(int *data, int *pattern, int n, int threshold) {
    #pragma omp simd simdlen(4) safelen(16) \
        if(n > threshold) aligned(data, pattern: 32)
    for (int i = 0; i < n; i++) {
        data[i] = (pattern[i] > 0) ? data[i] * 2 : data[i] / 2;
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(float *arr, int n) {
    double sum = 0.0;
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    /* Parse command line for GPU offloading flag */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    /* Allocate and initialize test data */
    float *a_f = (float*)aligned_alloc(64, N * sizeof(float));
    float *b_f = (float*)aligned_alloc(64, N * sizeof(float));
    float *c_f = (float*)aligned_alloc(64, N * sizeof(float));
    
    int *a_i = (int*)aligned_alloc(64, N * sizeof(int));
    int *b_i = (int*)aligned_alloc(64, N * sizeof(int));
    int *c_i = (int*)aligned_alloc(64, N * sizeof(int));
    
    double *a_d = (double*)aligned_alloc(128, N * sizeof(double));
    double *b_d = (double*)aligned_alloc(128, N * sizeof(double));
    double *c_d = (double*)aligned_alloc(128, N * sizeof(double));
    
    /* Initialize with patterned data */
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 0.1f;
        b_f[i] = (N - i) * 0.2f;
        c_f[i] = 0.0f;
        
        a_i[i] = i % 100;
        b_i[i] = (i * 3) % 100;
        c_i[i] = 0;
        
        a_d[i] = i * 0.01;
        b_d[i] = (N - i) * 0.02;
        c_d[i] = 0.0;
    }
    
    printf("Starting OpenMP SIMD/SIMT tests...\n");
    
    /* Execute test functions */
    test_target_simd(a_f, b_f, c_f, N);
    double checksum1 = compute_checksum(c_f, N);
    printf("Test 1 checksum: %f\n", checksum1);
    
    test_parallel_for_simd(a_i, b_i, c_i, N);
    int sum_i = 0;
    #pragma omp simd reduction(+:sum_i)
    for (int i = 0; i < N; i++) sum_i += c_i[i];
    printf("Test 2 sum: %d\n", sum_i);
    
    test_nested_simd(a_d, b_d, c_d, N);
    double sum_d = 0.0;
    #pragma omp simd reduction(+:sum_d) simdlen(4)
    for (int i = 0; i < N; i++) sum_d += c_d[i];
    printf("Test 3 sum: %f\n", sum_d);
    
    test_mixed_directives(a_f, b_f, c_f, N);
    double checksum4 = compute_checksum(c_f, N);
    printf("Test 4 checksum: %f\n", checksum4);
    
    test_simd_with_if(a_i, b_i, N, 512);
    int final_sum = 0;
    #pragma omp simd reduction(+:final_sum) simdlen(8)
    for (int i = 0; i < N; i++) final_sum += a_i[i];
    printf("Test 5 final sum: %d\n", final_sum);
    
    /* Validation - compare with reference if both paths executed */
    if (use_gpu_offload) {
        /* Re-run test 1 on host for comparison */
        float *c_ref = (float*)aligned_alloc(64, N * sizeof(float));
        int save_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        test_target_simd(a_f, b_f, c_ref, N);
        
        int errors = 0;
        #pragma omp simd reduction(+:errors) simdlen(4)
        for (int i = 0; i < N; i++) {
            if (fabs(c_f[i] - c_ref[i]) > 1e-5) errors++;
        }
        
        if (errors == 0) {
            printf("Validation PASSED: GPU and CPU results match\n");
        } else {
            printf("Validation FAILED: %d mismatches found\n", errors);
        }
        
        free(c_ref);
        use_gpu_offload = save_flag;
    }
    
    /* Cleanup */
    free(a_f); free(b_f); free(c_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    
    return 0;
}
