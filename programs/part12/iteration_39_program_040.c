/* test_simt_transformation.c
 * Designed to trigger SIMT transformation in omp-low.cc lines 2941-2975
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 512
#define CHUNK_SIZE 64

/* Global flag to control offloading */
static int use_gpu_offload = 0;

/* Declare SIMD function */
#pragma omp declare simd uniform(a, b) linear(i:1) notinbranch
float simd_func(float a, float b, int i) {
    return a * b + i * 0.5f;
}

/* Test 1: Target teams distribute parallel for simd with conditional execution */
void test_target_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        num_teams(8) thread_limit(128) \
        simdlen(4) safelen(8) \
        private(n) aligned(a, b, c: 32)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) \
        aligned(a, b, c: 64) \
        schedule(static, CHUNK_SIZE) \
        private(n) \
        reduction(+:c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] += a[i] * b[i];
    }
}

/* Test 3: Nested loops with collapse */
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) \
            simdlen(4) \
            linear(i, j:1) \
            private(n, m)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] - b[idx];
            }
        }
    }
}

/* Test 4: SIMD with declare simd function call */
void test_declare_simd(float *a, float *b, float *c, int n) {
    #pragma omp target simd \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(4)
    for (int i = 0; i < n; i++) {
        c[i] = simd_func(a[i], b[i], i);
    }
}

/* Test 5: Mixed directives - simd inside for */
void test_mixed_directives(double *a, double *b, double *c, int n) {
    #pragma omp target teams distribute parallel for \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        num_teams(4)
    for (int i = 0; i < n; i++) {
        #pragma omp simd \
            simdlen(2) safelen(4) \
            aligned(a, b, c: 16)
        for (int j = 0; j < CHUNK_SIZE; j++) {
            int idx = i * CHUNK_SIZE + j;
            if (idx < n) {
                c[idx] = a[idx] / (b[idx] + 1.0);
            }
        }
    }
}

/* Test 6: SIMD with linear and reduction clauses */
float test_simd_reduction(float *a, int n) {
    float sum = 0.0f;
    
    #pragma omp simd \
        reduction(+:sum) \
        linear(i:1) \
        simdlen(8) \
        safelen(32)
    for (int i = 0; i < n; i++) {
        sum += a[i];
    }
    
    return sum;
}

/* Initialize arrays with patterned data */
void init_arrays(float *a, float *b, int *ia, int *ib, int n) {
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < n; i++) {
        a[i] = i * 1.5f;
        b[i] = (n - i) * 0.75f;
        ia[i] = i % 100;
        ib[i] = (i + 1) % 100;
    }
}

/* Compute checksum to prevent dead code elimination */
float compute_checksum(float *arr, int n) {
    float sum = 0.0f;
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
    
    /* Allocate arrays with different alignments */
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c1 = (float*)aligned_alloc(64, N * sizeof(float));
    float *c2 = (float*)aligned_alloc(64, N * sizeof(float));
    float *c3 = (float*)aligned_alloc(64, N * M * sizeof(float));
    float *c4 = (float*)aligned_alloc(64, N * sizeof(float));
    double *da = (double*)aligned_alloc(32, N * sizeof(double));
    double *db = (double*)aligned_alloc(32, N * sizeof(double));
    double *dc = (double*)aligned_alloc(32, N * sizeof(double));
    
    int *ia = (int*)malloc(N * sizeof(int));
    int *ib = (int*)malloc(N * sizeof(int));
    int *ic = (int*)calloc(N, sizeof(int));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !da || !db || !dc || !ia || !ib || !ic) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(a, b, ia, ib, N);
    for (int i = 0; i < N; i++) {
        da[i] = i * 0.25;
        db[i] = (N - i) * 0.5;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("\nTest 1: Target teams distribute parallel for simd\n");
    memset(c1, 0, N * sizeof(float));
    test_target_simd(a, b, c1, N);
    printf("Checksum 1: %f\n", compute_checksum(c1, N));
    
    /* Test 2: Parallel for simd */
    printf("\nTest 2: Parallel for simd\n");
    test_parallel_for_simd(ia, ib, ic, N);
    int sum_int = 0;
    for (int i = 0; i < N; i++) sum_int += ic[i];
    printf("Integer sum: %d\n", sum_int);
    
    /* Test 3: Nested SIMD */
    printf("\nTest 3: Nested SIMD with collapse\n");
    memset(c3, 0, N * M * sizeof(float));
    test_nested_simd(a, b, c3, N, M);
    printf("Checksum 3: %f\n", compute_checksum(c3, N * M));
    
    /* Test 4: Declare SIMD function */
    printf("\nTest 4: SIMD with declare simd function\n");
    memset(c4, 0, N * sizeof(float));
    test_declare_simd(a, b, c4, N);
    printf("Checksum 4: %f\n", compute_checksum(c4, N));
    
    /* Test 5: Mixed directives */
    printf("\nTest 5: Mixed directives (simd inside for)\n");
    memset(dc, 0, N * sizeof(double));
    test_mixed_directives(da, db, dc, N);
    double sum_double = 0.0;
    for (int i = 0; i < N; i++) sum_double += dc[i];
    printf("Double sum: %f\n", sum_double);
    
    /* Test 6: SIMD reduction */
    printf("\nTest 6: SIMD reduction\n");
    float reduction_sum = test_simd_reduction(a, N);
    printf("Reduction sum: %f\n", reduction_sum);
    
    /* Validation: Compare GPU and CPU paths if both were executed */
    if (use_gpu_offload) {
        printf("\nGPU offloading path was executed\n");
        
        /* Run host-only version for comparison */
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c1_host = (float*)aligned_alloc(64, N * sizeof(float));
        memset(c1_host, 0, N * sizeof(float));
        test_target_simd(a, b, c1_host, N);
        
        /* Compare results */
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (fabs(c1[i] - c1_host[i]) > 1e-6) {
                errors++;
                if (errors < 5) {
                    printf("Mismatch at index %d: GPU=%f, CPU=%f\n", 
                           i, c1[i], c1_host[i]);
                }
            }
        }
        
        if (errors == 0) {
            printf("Validation PASSED: GPU and CPU results match\n");
        } else {
            printf("Validation FAILED: %d mismatches found\n", errors);
        }
        
        free(c1_host);
        use_gpu_offload = saved_flag;
    }
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4);
    free(da); free(db); free(dc);
    free(ia); free(ib); free(ic);
    
    printf("\nAll tests completed\n");
    return 0;
}
