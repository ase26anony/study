#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

static int use_gpu_offload = 0;

/* Declare SIMD function for vector addition */
#pragma omp declare simd uniform(a, b) linear(i:1) simdlen(8)
float simd_add(float a, float b, int i) {
    return a + b + (i * 0.001f);
}

/* Test 1: Target teams distribute parallel for simd with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(4) safelen(8) aligned(a, b, c: 32) \
        private(n) reduction(+:n)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] + simd_add(a[i], b[i], i);
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c: 64) \
        linear(i:1) private(i)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] - (i % 16);
    }
}

/* Test 3: Nested loops with collapse */
void test_nested_simd(double *a, double *b, double *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(2) safelen(4)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * 2.5 + b[idx] / 3.0;
            }
        }
    }
}

/* Test 4: Mixed directives - simd inside for */
void test_mixed_simd(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int block = 0; block < n; block += 64) {
            int end = (block + 64 < n) ? block + 64 : n;
            #pragma omp simd simdlen(4) aligned(a, b, c: 16)
            for (int i = block; i < end; i++) {
                c[i] = (a[i] - b[i]) * (a[i] + b[i]);
            }
        }
    }
}

/* Test 5: SIMD with linear clause variations */
void test_linear_simd(int *a, int *b, int *c, int n) {
    int k = 3;
    #pragma omp simd linear(k:2) simdlen(4)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] + k;
        k += 2;  // Linear increment
    }
}

/* Helper function to compute checksum */
double compute_checksum(float *arr, int n) {
    double sum = 0.0;
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
    
    /* Allocate and initialize arrays with dynamic allocation */
    float *a_f = (float *)aligned_alloc(64, N * sizeof(float));
    float *b_f = (float *)aligned_alloc(64, N * sizeof(float));
    float *c_f = (float *)aligned_alloc(64, N * sizeof(float));
    
    int *a_i = (int *)aligned_alloc(64, N * sizeof(int));
    int *b_i = (int *)aligned_alloc(64, N * sizeof(int));
    int *c_i = (int *)aligned_alloc(64, N * sizeof(int));
    
    double *a_d = (double *)aligned_alloc(64, N * M * sizeof(double));
    double *b_d = (double *)aligned_alloc(64, N * M * sizeof(double));
    double *c_d = (double *)aligned_alloc(64, N * M * sizeof(double));
    
    /* Initialize with patterned data */
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 0.5f;
        b_f[i] = (N - i) * 0.3f;
        c_f[i] = 0.0f;
        
        a_i[i] = i;
        b_i[i] = N - i;
        c_i[i] = 0;
    }
    
    #pragma omp parallel for simd collapse(2) simdlen(4)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            a_d[idx] = i * 0.25 + j * 0.1;
            b_d[idx] = (N - i) * 0.15 + (M - j) * 0.05;
            c_d[idx] = 0.0;
        }
    }
    
    /* Execute test functions */
    printf("Running test_target_simd...\n");
    test_target_simd(a_f, b_f, c_f, N);
    double checksum1 = compute_checksum(c_f, N);
    printf("Checksum 1: %f\n", checksum1);
    
    printf("Running test_parallel_for_simd...\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    double sum_i = 0.0;
    #pragma omp simd reduction(+:sum_i) simdlen(8)
    for (int i = 0; i < N; i++) sum_i += c_i[i];
    printf("Checksum 2: %f\n", sum_i);
    
    printf("Running test_nested_simd...\n");
    test_nested_simd(a_d, b_d, c_d, N, M);
    double sum_d = 0.0;
    #pragma omp simd reduction(+:sum_d) simdlen(4)
    for (int i = 0; i < N * M; i++) sum_d += c_d[i];
    printf("Checksum 3: %f\n", sum_d);
    
    printf("Running test_mixed_simd...\n");
    test_mixed_simd(a_f, b_f, c_f, N);
    double checksum4 = compute_checksum(c_f, N);
    printf("Checksum 4: %f\n", checksum4);
    
    printf("Running test_linear_simd...\n");
    test_linear_simd(a_i, b_i, c_i, N);
    sum_i = 0.0;
    #pragma omp simd reduction(+:sum_i) simdlen(8)
    for (int i = 0; i < N; i++) sum_i += c_i[i];
    printf("Checksum 5: %f\n", sum_i);
    
    /* Validation: Run both paths if GPU offloading was enabled */
    if (use_gpu_offload) {
        printf("\nRunning host-only comparison...\n");
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c_host = (float *)aligned_alloc(64, N * sizeof(float));
        memset(c_host, 0, N * sizeof(float));
        
        test_target_simd(a_f, b_f, c_host, N);
        
        /* Compare results */
        int errors = 0;
        #pragma omp simd reduction(+:errors) simdlen(8)
        for (int i = 0; i < N; i++) {
            if (fabs(c_f[i] - c_host[i]) > 1e-5) errors++;
        }
        
        if (errors == 0) {
            printf("GPU and host results match!\n");
        } else {
            printf("Mismatch found in %d elements\n", errors);
        }
        
        free(c_host);
        use_gpu_offload = saved_flag;
    }
    
    /* Cleanup */
    free(a_f); free(b_f); free(c_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    
    return 0;
}
