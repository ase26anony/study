/* test_simt_transformation.c
 * Designed to trigger SIMT transformation in omp-low.cc lines 2941-2975
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
#pragma omp declare simd uniform(a, b) linear(i:1) simdlen(4)
float vector_add_element(float a, float b, int i) {
    return a + b + (i * 0.001f);
}

/* Test 1: Target teams distribute parallel for simd with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(8) safelen(16) collapse(1) \
            private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    } else {
        /* Host fallback with similar SIMD construct */
        #pragma omp simd simdlen(4) aligned(a, b, c: 32) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(4) safelen(8) \
        aligned(a, b, c: 16) private(i) schedule(static)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

/* Test 3: Nested loops with collapse clause */
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(2) private(i, j)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] - b[idx];
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
                c[idx] = a[idx] / (b[idx] + 1.0);
            }
        }
    }
}

/* Test 5: Using declare simd function in target region */
void test_declare_simd_function(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4)
        for (int i = 0; i < n; i++) {
            c[i] = vector_add_element(a[i], b[i], i);
        }
    }
}

/* Test 6: Complex reduction with SIMD */
float test_simd_reduction(float *data, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) simdlen(8) safelen(16) \
        aligned(data: 64) linear(i:1)
    for (int i = 0; i < n; i++) {
        sum += data[i] * data[i];
    }
    
    return sum;
}

/* Test 7: Dynamic data with pointer aliasing */
void test_dynamic_data(int n) {
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    
    /* Initialize with pattern */
    #pragma omp simd simdlen(4)
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
    }
    
    /* Process with target simd if GPU offload enabled */
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) collapse(1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2;
        }
    } else {
        #pragma omp parallel for simd simdlen(8)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2;
        }
    }
    
    /* Checksum to prevent dead code elimination */
    int checksum = 0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < n; i++) {
        checksum += c[i];
    }
    printf("Dynamic data checksum: %d\n", checksum);
    
    free(a);
    free(b);
    free(c);
}

int main(int argc, char *argv[]) {
    /* Parse command line argument for GPU offloading */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    /* Set environment variable to influence runtime decision */
    if (use_gpu_offload) {
        setenv("OMP_TARGET_OFFLOAD", "MANDATORY", 1);
    }
    
    /* Allocate and initialize test arrays */
    float *fa = (float*)aligned_alloc(64, N * sizeof(float));
    float *fb = (float*)aligned_alloc(64, N * sizeof(float));
    float *fc = (float*)aligned_alloc(64, N * sizeof(float));
    
    int *ia = (int*)aligned_alloc(32, N * sizeof(int));
    int *ib = (int*)aligned_alloc(32, N * sizeof(int));
    int *ic = (int*)aligned_alloc(32, N * sizeof(int));
    
    double *da = (double*)aligned_alloc(128, N * 4 * sizeof(double));
    double *db = (double*)aligned_alloc(128, N * 4 * sizeof(double));
    double *dc = (double*)aligned_alloc(128, N * 4 * sizeof(double));
    
    /* Initialize with patterns */
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < N; i++) {
        fa[i] = i * 1.5f;
        fb[i] = N - i * 0.5f;
        ia[i] = i * 2;
        ib[i] = i * 3;
    }
    
    for (int i = 0; i < N * 4; i++) {
        da[i] = i * 0.25;
        db[i] = i * 0.75;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Run all test functions to trigger various SIMT paths */
    
    /* Test 1: Conditional target SIMD */
    test_target_simd(fa, fb, fc, N);
    
    /* Compute checksum */
    float sum1 = 0.0f;
    #pragma omp simd reduction(+:sum1)
    for (int i = 0; i < N; i++) {
        sum1 += fc[i];
    }
    printf("Test 1 checksum: %.2f\n", sum1);
    
    /* Test 2: Parallel for SIMD */
    test_parallel_for_simd(ia, ib, ic, N);
    
    int sum2 = 0;
    #pragma omp simd reduction(+:sum2)
    for (int i = 0; i < N; i++) {
        sum2 += ic[i];
    }
    printf("Test 2 checksum: %d\n", sum2);
    
    /* Test 3: Nested SIMD */
    float *fnested = (float*)aligned_alloc(32, N * M * sizeof(float));
    test_nested_simd(fa, fb, fnested, 32, 32);
    
    /* Test 4: Mixed directives */
    test_mixed_directives(da, db, dc, N);
    
    /* Test 5: Declare SIMD function */
    if (use_gpu_offload) {
        test_declare_simd_function(fa, fb, fc, N);
    }
    
    /* Test 6: SIMD reduction */
    float reduction_result = test_simd_reduction(fa, N);
    printf("Test 6 reduction result: %.2f\n", reduction_result);
    
    /* Test 7: Dynamic data with aliasing */
    test_dynamic_data(256);
    
    /* Validation: Compare host-only vs GPU results if both paths taken */
    if (use_gpu_offload) {
        /* Run host version for comparison */
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *fc_host = (float*)aligned_alloc(64, N * sizeof(float));
        test_target_simd(fa, fb, fc_host, N);
        
        /* Compare results */
        int errors = 0;
        #pragma omp simd reduction(+:errors)
        for (int i = 0; i < N; i++) {
            if (fabsf(fc[i] - fc_host[i]) > 0.001f) {
                errors++;
            }
        }
        
        if (errors == 0) {
            printf("Validation PASSED: GPU and host results match\n");
        } else {
            printf("Validation FAILED: %d mismatches found\n", errors);
        }
        
        free(fc_host);
        use_gpu_offload = saved_flag;
    }
    
    /* Cleanup */
    free(fa);
    free(fb);
    free(fc);
    free(ia);
    free(ib);
    free(ic);
    free(da);
    free(db);
    free(dc);
    free(fnested);
    
    printf("All tests completed\n");
    return 0;
}
