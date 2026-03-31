#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

/* Global flag to control offloading */
static int use_gpu_offload = 0;

/* Function with declare simd for vectorization */
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_add(float a, float b, int i) {
    return a + b + (i * 0.001f);
}

/* Test 1: Target teams distribute parallel for simd with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(64) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) aligned(a, b, c: 32) \
            private(n) reduction(+:n)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_add(a[i], b[i], i);
        }
    } else {
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(8) safelen(16) \
        schedule(static, 64) collapse(2) \
        aligned(a, b, c: 64) private(n)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            if (idx < n) {
                c[idx] = a[idx] * 2 + b[idx] / 3;
            }
        }
    }
}

/* Test 3: Nested SIMD with collapse clause */
void test_nested_simd(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(2) \
            aligned(a, b, c: 16) linear(i, j:1)
        for (int i = 0; i < n/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                c[idx] = a[idx] * b[idx] - (double)(i + j) * 0.5;
            }
        }
    }
}

/* Test 4: Mixed directives - simd inside parallel region */
void test_mixed_simd(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int chunk = 0; chunk < n; chunk += 128) {
            int end = (chunk + 128 < n) ? chunk + 128 : n;
            #pragma omp simd simdlen(4) aligned(a, b, c: 16)
            for (int i = chunk; i < end; i++) {
                c[i] = (a[i] - b[i]) * (a[i] + b[i]);
            }
        }
    }
}

/* Test 5: SIMD with reduction and private clauses */
float test_simd_reduction(float *a, float *b, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) simdlen(8) \
        private(n) aligned(a, b: 32) linear(i:1)
    for (int i = 0; i < n; i++) {
        sum += a[i] * b[i] + (float)i * 0.01f;
    }
    
    return sum;
}

int main(int argc, char **argv) {
    /* Parse command line argument for GPU offloading */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    /* Allocate and initialize arrays */
    float *a_f = (float*)aligned_alloc(32, N * sizeof(float));
    float *b_f = (float*)aligned_alloc(32, N * sizeof(float));
    float *c_f = (float*)aligned_alloc(32, N * sizeof(float));
    
    int *a_i = (int*)aligned_alloc(64, N * sizeof(int));
    int *b_i = (int*)aligned_alloc(64, N * sizeof(int));
    int *c_i = (int*)aligned_alloc(64, N * sizeof(int));
    
    double *a_d = (double*)aligned_alloc(16, N * sizeof(double));
    double *b_d = (double*)aligned_alloc(16, N * sizeof(double));
    double *c_d = (double*)aligned_alloc(16, N * sizeof(double));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; i++) {
        a_f[i] = (float)i;
        b_f[i] = (float)(N - i);
        c_f[i] = 0.0f;
        
        a_i[i] = i % 100;
        b_i[i] = (i * 3) % 100;
        c_i[i] = 0;
        
        a_d[i] = (double)i * 0.5;
        b_d[i] = (double)(N - i) * 0.5;
        c_d[i] = 0.0;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("\nTest 1: Target SIMD\n");
    test_target_simd(a_f, b_f, c_f, N);
    
    /* Compute checksum */
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N; i++) {
        checksum1 += c_f[i];
    }
    printf("Checksum 1: %f\n", checksum1);
    
    /* Test 2: Parallel for SIMD */
    printf("\nTest 2: Parallel for SIMD\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    int checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N; i++) {
        checksum2 += c_i[i];
    }
    printf("Checksum 2: %d\n", checksum2);
    
    /* Test 3: Nested SIMD */
    printf("\nTest 3: Nested SIMD\n");
    test_nested_simd(a_d, b_d, c_d, N);
    
    double checksum3 = 0.0;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < N; i++) {
        checksum3 += c_d[i];
    }
    printf("Checksum 3: %f\n", checksum3);
    
    /* Test 4: Mixed directives */
    printf("\nTest 4: Mixed directives\n");
    test_mixed_simd(a_f, b_f, c_f, N);
    
    float checksum4 = 0.0f;
    #pragma omp simd reduction(+:checksum4)
    for (int i = 0; i < N; i++) {
        checksum4 += c_f[i];
    }
    printf("Checksum 4: %f\n", checksum4);
    
    /* Test 5: SIMD reduction */
    printf("\nTest 5: SIMD reduction\n");
    float reduction_result = test_simd_reduction(a_f, b_f, N);
    printf("Reduction result: %f\n", reduction_result);
    
    /* Validation: Compare host-only path if GPU was used */
    if (use_gpu_offload) {
        printf("\nValidating GPU vs CPU results...\n");
        
        /* Run host-only version for comparison */
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c_f_host = (float*)aligned_alloc(32, N * sizeof(float));
        memset(c_f_host, 0, N * sizeof(float));
        
        test_target_simd(a_f, b_f, c_f_host, N);
        
        /* Compare results */
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (fabs(c_f[i] - c_f_host[i]) > 0.001f) {
                errors++;
                if (errors < 5) {
                    printf("Mismatch at index %d: GPU=%f, CPU=%f\n", 
                           i, c_f[i], c_f_host[i]);
                }
            }
        }
        
        if (errors == 0) {
            printf("Validation PASSED: GPU and CPU results match\n");
        } else {
            printf("Validation FAILED: %d mismatches found\n", errors);
        }
        
        free(c_f_host);
        use_gpu_offload = saved_flag;
    }
    
    /* Cleanup */
    free(a_f); free(b_f); free(c_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    
    return 0;
}
