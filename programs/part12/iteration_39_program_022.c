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

/* Global flag to control offloading at runtime */
static int use_gpu_offload = 0;

/* Declare SIMD function for testing declare simd directive */
#pragma omp declare simd uniform(b) linear(i:1) notinbranch
float simd_func(float a, float b, int i) {
    return a * b + i * 0.5f;
}

/* Test 1: Target teams distribute parallel for simd with conditional execution */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(128) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(8) safelen(16) aligned(a, b, c: 32) \
            private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        /* Host fallback version */
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 16) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(4) safelen(8) collapse(2) \
        schedule(static, 16) private(i, j) \
        aligned(a, b, c: 64) linear(j:1)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c[idx] = a[idx] * b[idx] + i - j;
        }
    }
}

/* Test 3: Nested SIMD with collapse */
void test_nested_simd(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) \
            simdlen(2) safelen(4) \
            aligned(a, b, c: 128) \
            private(i, j, k)
        for (int i = 0; i < n/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                double sum = 0.0;
                #pragma omp simd reduction(+:sum) simdlen(4)
                for (int k = 0; k < 8; k++) {
                    sum += a[idx * 8 + k] * b[k];
                }
                c[idx] = sum;
            }
        }
    }
}

/* Test 4: SIMD function calls within target region */
void test_simd_function_calls(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b) map(from: c[0:n]) \
            simdlen(8) num_teams(2)
        for (int i = 0; i < n; i++) {
            /* Call declared SIMD function */
            c[i] = simd_func(a[i], b[0], i);
        }
    }
}

/* Test 5: Mixed directives - simd inside for */
void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i += 64) {
            int end = (i + 64 < n) ? i + 64 : n;
            #pragma omp simd simdlen(8) safelen(16) aligned(a, b, c: 32)
            for (int j = i; j < end; j++) {
                c[j] = a[j] - b[j];
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
                printf("Mismatch at %d: %f != %f\n", i, c1[i], c2[i]);
            }
        }
    }
    return errors;
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
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c1 = (float*)aligned_alloc(64, N * sizeof(float));
    float *c2 = (float*)aligned_alloc(64, N * sizeof(float));
    float *c3 = (float*)aligned_alloc(64, N * sizeof(float));
    
    int *ia = (int*)aligned_alloc(64, N * M * sizeof(int));
    int *ib = (int*)aligned_alloc(64, N * M * sizeof(int));
    int *ic = (int*)aligned_alloc(64, N * M * sizeof(int));
    
    double *da = (double*)aligned_alloc(128, N * sizeof(double));
    double *db = (double*)aligned_alloc(128, 8 * sizeof(double));
    double *dc = (double*)aligned_alloc(128, N * sizeof(double));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.5f;
        b[i] = N - i * 0.5f;
        c1[i] = c2[i] = c3[i] = 0.0f;
    }
    
    for (int i = 0; i < N * M; i++) {
        ia[i] = i % 100;
        ib[i] = (i * 3) % 100;
        ic[i] = 0;
    }
    
    for (int i = 0; i < N; i++) {
        da[i] = i * 0.25;
    }
    for (int i = 0; i < 8; i++) {
        db[i] = i * 0.1;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("\nTest 1: Target SIMD with conditional execution\n");
    test_target_simd(a, b, c1, N);
    
    /* Compute checksum to prevent dead code elimination */
    float sum1 = 0.0f;
    #pragma omp simd reduction(+:sum1)
    for (int i = 0; i < N; i++) {
        sum1 += c1[i];
    }
    printf("Checksum 1: %f\n", sum1);
    
    /* Test 2: Parallel for SIMD */
    printf("\nTest 2: Parallel for SIMD with collapse\n");
    test_parallel_for_simd(ia, ib, ic, N);
    
    int sum2 = 0;
    #pragma omp simd reduction(+:sum2)
    for (int i = 0; i < N * M; i++) {
        sum2 += ic[i];
    }
    printf("Checksum 2: %d\n", sum2);
    
    /* Test 3: Nested SIMD */
    printf("\nTest 3: Nested SIMD with collapse\n");
    test_nested_simd(da, db, dc, N);
    
    double sum3 = 0.0;
    #pragma omp simd reduction(+:sum3)
    for (int i = 0; i < N; i++) {
        sum3 += dc[i];
    }
    printf("Checksum 3: %f\n", sum3);
    
    /* Test 4: SIMD function calls */
    printf("\nTest 4: SIMD function calls in target region\n");
    test_simd_function_calls(a, b, c2, N);
    
    float sum4 = 0.0f;
    #pragma omp simd reduction(+:sum4)
    for (int i = 0; i < N; i++) {
        sum4 += c2[i];
    }
    printf("Checksum 4: %f\n", sum4);
    
    /* Test 5: Mixed directives */
    printf("\nTest 5: Mixed directives (simd inside for)\n");
    test_mixed_directives(a, b, c3, N);
    
    float sum5 = 0.0f;
    #pragma omp simd reduction(+:sum5)
    for (int i = 0; i < N; i++) {
        sum5 += c3[i];
    }
    printf("Checksum 5: %f\n", sum5);
    
    /* Validation between different execution paths */
    if (use_gpu_offload) {
        /* Run host-only version for comparison */
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c_host = (float*)aligned_alloc(64, N * sizeof(float));
        memset(c_host, 0, N * sizeof(float));
        
        /* Re-run test 1 on host */
        #pragma omp simd simdlen(4) aligned(a, b, c_host: 16)
        for (int i = 0; i < N; i++) {
            c_host[i] = a[i] + b[i] * 2.0f;
        }
        
        /* Compare results */
        int errors = validate_results(c1, c_host, N, 0.001f);
        if (errors == 0) {
            printf("\nValidation PASSED: GPU and CPU results match\n");
        } else {
            printf("\nValidation FAILED: %d mismatches found\n", errors);
        }
        
        free(c_host);
        use_gpu_offload = saved_flag;
    }
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3);
    free(ia); free(ib); free(ic);
    free(da); free(db); free(dc);
    
    printf("\nAll tests completed\n");
    return 0;
}
