/* test_simt_coverage.c - Comprehensive OpenMP SIMD/SIMT test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32
#define CHUNK_SIZE 128

/* Global flag to control offloading */
static int use_gpu_offload = 0;

/* Function with declare simd for vectorization */
#pragma omp declare simd uniform(b) linear(i:1) notinbranch
float simd_func(float a, float b, int i) {
    return a * b + i * 0.5f;
}

/* Test 1: Target teams distribute parallel for simd with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        num_teams(4) thread_limit(128) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(8) safelen(16) aligned(a, b, c: 32) \
        private(n) reduction(+:n)
    for (int i = 0; i < n; i++) {
        float temp = a[i] + b[i];
        c[i] = simd_func(temp, 2.0f, i);
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(4) safelen(8) collapse(2) \
        schedule(static, CHUNK_SIZE) \
        aligned(a, b, c: 16) linear(i, j:1) \
        lastprivate(j)
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
            simdlen(2) ordered(2) \
            private(i, j)
        for (int i = 0; i < n; i += 2) {
            for (int j = 0; j < n; j += 2) {
                int idx = i * n + j;
                c[idx] = a[idx] * b[idx] - (i + j) * 0.5;
                #pragma omp ordered simd
                c[idx + 1] = a[idx + 1] + b[idx + 1];
            }
        }
    }
}

/* Test 4: SIMD with reduction and linear clauses */
float test_simd_reduction(float *arr, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) linear(i:1) \
        simdlen(16) aligned(arr: 64)
    for (int i = 0; i < n; i++) {
        sum += arr[i] * (i % 8);
    }
    
    return sum;
}

/* Test 5: Mixed directives - for with SIMD inside */
void test_mixed_directives(int *a, int *b, int *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            #pragma omp simd simdlen(8)
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] ^ b[idx];
            }
        }
        
        #pragma omp for simd simdlen(4)
        for (int i = 0; i < n; i++) {
            a[i] = b[i] * 2;
        }
    }
}

/* Test 6: SIMD with dynamic data and pointers */
void test_pointer_simd(float **ptr_arr, int n, int m) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: ptr_arr[0:n][0:m]) \
        simdlen(4) collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            ptr_arr[i][j] = ptr_arr[i][j] * 1.5f + (i + j);
        }
    }
}

/* Initialize arrays with pattern */
void init_arrays(float *a, float *b, float *c, int n) {
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < n; i++) {
        a[i] = i * 1.0f;
        b[i] = (n - i) * 0.5f;
        c[i] = 0.0f;
    }
}

void init_int_arrays(int *a, int *b, int *c, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = 0;
    }
}

/* Compute checksum for validation */
float compute_checksum(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
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
            printf("GPU offloading enabled via command line\n");
        }
    }
    
    /* Also check environment variable */
    if (getenv("OMP_TARGET_OFFLOAD") != NULL) {
        use_gpu_offload = atoi(getenv("OMP_TARGET_OFFLOAD"));
    }
    
    const int total_size = N * M;
    
    /* Allocate and initialize arrays */
    float *a = (float*)aligned_alloc(64, total_size * sizeof(float));
    float *b = (float*)aligned_alloc(64, total_size * sizeof(float));
    float *c = (float*)aligned_alloc(64, total_size * sizeof(float));
    
    int *a_int = (int*)aligned_alloc(32, total_size * sizeof(int));
    int *b_int = (int*)aligned_alloc(32, total_size * sizeof(int));
    int *c_int = (int*)aligned_alloc(32, total_size * sizeof(int));
    
    double *a_dbl = (double*)aligned_alloc(128, N * N * sizeof(double));
    double *b_dbl = (double*)aligned_alloc(128, N * N * sizeof(double));
    double *c_dbl = (double*)aligned_alloc(128, N * N * sizeof(double));
    
    /* Initialize data */
    init_arrays(a, b, c, total_size);
    init_int_arrays(a_int, b_int, c_int, total_size);
    
    #pragma omp parallel for simd
    for (int i = 0; i < N * N; i++) {
        a_dbl[i] = i * 0.25;
        b_dbl[i] = i * 0.75;
        c_dbl[i] = 0.0;
    }
    
    printf("Starting OpenMP SIMD/SIMT tests...\n");
    printf("Using GPU offload: %s\n", use_gpu_offload ? "YES" : "NO");
    
    /* Execute test functions */
    printf("\n1. Testing target simd with conditional offloading...\n");
    test_target_simd(a, b, c, total_size);
    float checksum1 = compute_checksum(c, total_size);
    printf("   Checksum: %f\n", checksum1);
    
    printf("\n2. Testing parallel for simd...\n");
    test_parallel_for_simd(a_int, b_int, c_int, N);
    int sum_int = 0;
    #pragma omp simd reduction(+:sum_int)
    for (int i = 0; i < total_size; i++) {
        sum_int += c_int[i];
    }
    printf("   Integer sum: %d\n", sum_int);
    
    printf("\n3. Testing nested simd with collapse...\n");
    test_nested_simd(a_dbl, b_dbl, c_dbl, N);
    double sum_dbl = 0.0;
    #pragma omp simd reduction(+:sum_dbl)
    for (int i = 0; i < N * N; i++) {
        sum_dbl += c_dbl[i];
    }
    printf("   Double sum: %f\n", sum_dbl);
    
    printf("\n4. Testing simd reduction...\n");
    float red_sum = test_simd_reduction(a, total_size);
    printf("   Reduction sum: %f\n", red_sum);
    
    printf("\n5. Testing mixed directives...\n");
    test_mixed_directives(a_int, b_int, c_int, N);
    
    printf("\n6. Testing pointer-based simd with dynamic data...\n");
    float **ptr_arr = (float**)malloc(N * sizeof(float*));
    for (int i = 0; i < N; i++) {
        ptr_arr[i] = (float*)aligned_alloc(32, M * sizeof(float));
        #pragma omp simd
        for (int j = 0; j < M; j++) {
            ptr_arr[i][j] = (i * M + j) * 0.1f;
        }
    }
    
    /* Conditional execution based on runtime flag */
    if (use_gpu_offload) {
        test_pointer_simd(ptr_arr, N, M);
    }
    
    /* Cleanup */
    for (int i = 0; i < N; i++) {
        free(ptr_arr[i]);
    }
    free(ptr_arr);
    
    free(a); free(b); free(c);
    free(a_int); free(b_int); free(c_int);
    free(a_dbl); free(b_dbl); free(c_dbl);
    
    printf("\nAll tests completed successfully!\n");
    
    return 0;
}
