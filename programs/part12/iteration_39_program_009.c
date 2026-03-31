/* test_simt_transformation.c
 * Designed to trigger GCC's SIMT transformation in omp-low.cc lines 2941-2975
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

static int use_gpu_offload = 0;

/* Function with declare simd for vectorization */
#pragma omp declare simd uniform(a, b) linear(i) simdlen(4)
float simd_multiply(float a, float b, int i) {
    return a * b * (i % 10);
}

/* Test 1: Target teams distribute parallel for simd with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(8) safelen(16) collapse(1) \
            private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_multiply(a[i], b[i], i);
        }
    } else {
        /* Host fallback version */
        #pragma omp simd simdlen(4) aligned(a, b, c: 32) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(4) safelen(8) \
        aligned(a, b, c: 16) schedule(static) num_threads(4)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] - (i % 7);
    }
}

/* Test 3: Nested loops with collapse clause */
void test_nested_simd(double *a, double *b, double *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for collapse(2) simd simdlen(4)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * 2.5 + b[idx] / 3.0;
            }
        }
    }
}

/* Test 4: Mixed directives - simd inside for */
void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i += 4) {
        #pragma omp simd simdlen(4) aligned(a, b, c: 32)
        for (int j = 0; j < 4 && (i + j) < n; j++) {
            c[i + j] = a[i + j] - b[i + j];
        }
    }
}

/* Test 5: SIMD with linear and reduction clauses */
void test_simd_reduction(float *arr, int n, float *result) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) linear(i:1) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += arr[i] * (i % 5 + 1);
    }
    
    *result = sum;
}

/* Test 6: Complex target region with multiple nested constructs */
void test_complex_target(int *a, int *b, int *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(tofrom: c[0:n]) \
            simdlen(4) collapse(2) num_teams(8) thread_limit(128)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < n) {
                    c[idx] = a[idx] + b[idx] * (j + 1);
                }
            }
        }
    }
}

/* Validation function */
int validate_results(float *c1, float *c2, int n, float tolerance) {
    for (int i = 0; i < n; i++) {
        if (fabs(c1[i] - c2[i]) > tolerance) {
            printf("Mismatch at index %d: %f vs %f\n", i, c1[i], c2[i]);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    /* Parse command line argument for GPU offloading */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    /* Allocate and initialize arrays with dynamic allocation */
    float *a_f = (float*)malloc(N * sizeof(float));
    float *b_f = (float*)malloc(N * sizeof(float));
    float *c1_f = (float*)malloc(N * sizeof(float));
    float *c2_f = (float*)malloc(N * sizeof(float));
    float *c3_f = (float*)malloc(N * sizeof(float));
    
    int *a_i = (int*)malloc(N * sizeof(int));
    int *b_i = (int*)malloc(N * sizeof(int));
    int *c_i = (int*)malloc(N * sizeof(int));
    
    double *a_d = (double*)malloc(N * M * sizeof(double));
    double *b_d = (double*)malloc(N * M * sizeof(double));
    double *c_d = (double*)malloc(N * M * sizeof(double));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 1.5f;
        b_f[i] = N - i * 0.7f;
        c1_f[i] = 0.0f;
        c2_f[i] = 0.0f;
        c3_f[i] = 0.0f;
        
        a_i[i] = i % 100;
        b_i[i] = (i * 3) % 100;
        c_i[i] = 0;
    }
    
    for (int i = 0; i < N * M; i++) {
        a_d[i] = i * 0.25;
        b_d[i] = i * 0.75;
        c_d[i] = 0.0;
    }
    
    printf("Starting OpenMP SIMD/SIMT tests...\n");
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("\nTest 1: Target SIMD with conditional offloading\n");
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
    
    /* Test 3: Nested SIMD with collapse */
    printf("\nTest 3: Nested SIMD with collapse\n");
    test_nested_simd(a_d, b_d, c_d, N, M);
    
    double checksum3 = 0.0;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < N * M; i++) {
        checksum3 += c_d[i];
    }
    printf("Checksum 3: %f\n", checksum3);
    
    /* Test 4: Mixed directives */
    printf("\nTest 4: Mixed directives\n");
    test_mixed_directives(a_f, b_f, c2_f, N);
    
    float checksum4 = 0.0f;
    #pragma omp simd reduction(+:checksum4)
    for (int i = 0; i < N; i++) {
        checksum4 += c2_f[i];
    }
    printf("Checksum 4: %f\n", checksum4);
    
    /* Test 5: SIMD with reduction */
    printf("\nTest 5: SIMD with reduction\n");
    float reduction_result = 0.0f;
    test_simd_reduction(a_f, N, &reduction_result);
    printf("Reduction result: %f\n", reduction_result);
    
    /* Test 6: Complex target region */
    printf("\nTest 6: Complex target region\n");
    test_complex_target(a_i, b_i, c_i, N);
    
    /* Run host-only version for comparison if GPU was used */
    if (use_gpu_offload) {
        printf("\nRunning host-only version for comparison...\n");
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        /* Clear arrays */
        memset(c3_f, 0, N * sizeof(float));
        
        test_target_simd(a_f, b_f, c3_f, N);
        
        /* Validate results */
        if (validate_results(c1_f, c3_f, N, 0.001f)) {
            printf("GPU and CPU results match!\n");
        } else {
            printf("WARNING: GPU and CPU results differ!\n");
        }
        
        use_gpu_offload = saved_flag;
    }
    
    /* Cleanup */
    free(a_f);
    free(b_f);
    free(c1_f);
    free(c2_f);
    free(c3_f);
    free(a_i);
    free(b_i);
    free(c_i);
    free(a_d);
    free(b_d);
    free(c_d);
    
    printf("\nAll tests completed.\n");
    return 0;
}
