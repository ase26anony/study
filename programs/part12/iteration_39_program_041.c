#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64

/* Global flag to control GPU offloading */
static int use_gpu_offload = 0;

/* Function with declare simd for vectorization */
#pragma omp declare simd uniform(a, b) linear(i:1)
float compute_element(float a, float b, int i) {
    return a * b + (i % 10) * 0.1f;
}

/* Test 1: Target teams distribute parallel for simd with conditional execution */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + compute_element(a[i], b[i], i);
        }
    } else {
        /* Host fallback version */
        #pragma omp simd simdlen(4) aligned(a, b, c: 32) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + compute_element(a[i], b[i], i);
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(8) safelen(16) \
        aligned(a, b, c: 64) schedule(static, 32)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * 2 + b[i] / 3;
    }
}

/* Test 3: Nested loops with collapse clause */
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4) private(i, j)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * 0.5f + b[idx] * 2.0f;
            }
        }
    }
}

/* Test 4: Mixed directives - simd inside for */
void test_mixed_directives(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            #pragma omp simd simdlen(2)
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                c[idx] = a[idx] + b[idx] * (j + 1);
            }
        }
    }
}

/* Test 5: Complex target region with multiple clauses */
void test_complex_target(int *a, int *b, int *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(tofrom: c[0:n]) \
            num_teams(4) thread_limit(128) \
            simdlen(4) collapse(2) private(i, j) \
            reduction(max: max_val)
        for (int i = 0; i < n/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                c[idx] = a[idx] * b[idx] - (i + j);
                if (c[idx] > max_val) max_val = c[idx];
            }
        }
    }
}

/* Validation function */
int validate_results(float *c1, float *c2, int n, float tolerance) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (fabs(c1[i] - c2[i]) > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at index %d: %f != %f\n", i, c1[i], c2[i]);
            }
        }
    }
    return errors;
}

int main(int argc, char *argv[]) {
    /* Parse command line argument */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    /* Allocate and initialize arrays */
    float *a_f = (float*)aligned_alloc(64, N * sizeof(float));
    float *b_f = (float*)aligned_alloc(64, N * sizeof(float));
    float *c1_f = (float*)aligned_alloc(64, N * sizeof(float));
    float *c2_f = (float*)aligned_alloc(64, N * sizeof(float));
    
    int *a_i = (int*)malloc(N * sizeof(int));
    int *b_i = (int*)malloc(N * sizeof(int));
    int *c_i = (int*)malloc(N * sizeof(int));
    
    double *a_d = (double*)malloc(N * 4 * sizeof(double));
    double *b_d = (double*)malloc(N * 4 * sizeof(double));
    double *c_d = (double*)malloc(N * 4 * sizeof(double));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 0.1f;
        b_f[i] = (N - i) * 0.2f;
        c1_f[i] = 0.0f;
        c2_f[i] = 0.0f;
        
        a_i[i] = i;
        b_i[i] = N - i;
        c_i[i] = 0;
    }
    
    for (int i = 0; i < N * 4; i++) {
        a_d[i] = i * 0.01;
        b_d[i] = (N * 4 - i) * 0.02;
        c_d[i] = 0.0;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("\nTest 1: Target SIMD with conditional execution\n");
    test_target_simd(a_f, b_f, c1_f, N);
    
    /* Compute checksum to prevent dead code elimination */
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N; i++) {
        checksum1 += c1_f[i];
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
    printf("\nTest 3: Nested SIMD with collapse\n");
    test_nested_simd(a_f, b_f, c2_f, N/2, 2);
    
    float checksum3 = 0.0f;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < N; i++) {
        checksum3 += c2_f[i];
    }
    printf("Checksum 3: %f\n", checksum3);
    
    /* Test 4: Mixed directives */
    printf("\nTest 4: Mixed directives (simd inside for)\n");
    test_mixed_directives(a_d, b_d, c_d, N);
    
    double checksum4 = 0.0;
    #pragma omp simd reduction(+:checksum4)
    for (int i = 0; i < N * 4; i++) {
        checksum4 += c_d[i];
    }
    printf("Checksum 4: %lf\n", checksum4);
    
    /* Test 5: Complex target region */
    printf("\nTest 5: Complex target region\n");
    test_complex_target(a_i, b_i, c_i, N);
    
    /* Run host-only version for comparison if GPU was used */
    if (use_gpu_offload) {
        printf("\nRunning host-only version for comparison...\n");
        int original_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c_host = (float*)aligned_alloc(64, N * sizeof(float));
        memset(c_host, 0, N * sizeof(float));
        test_target_simd(a_f, b_f, c_host, N);
        
        /* Validate results */
        int errors = validate_results(c1_f, c_host, N, 0.001f);
        if (errors == 0) {
            printf("Validation PASSED: GPU and CPU results match\n");
        } else {
            printf("Validation FAILED: %d mismatches found\n", errors);
        }
        
        free(c_host);
        use_gpu_offload = original_flag;
    }
    
    /* Cleanup */
    free(a_f);
    free(b_f);
    free(c1_f);
    free(c2_f);
    free(a_i);
    free(b_i);
    free(c_i);
    free(a_d);
    free(b_d);
    free(c_d);
    
    printf("\nAll tests completed.\n");
    return 0;
}
