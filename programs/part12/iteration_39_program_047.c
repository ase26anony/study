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

/* Global flag to control offloading */
static int use_gpu_offload = 0;

/* Function with declare simd for vectorization */
#pragma omp declare simd uniform(a, b) linear(i:1) simdlen(4)
float compute_element(float a, float b, int i) {
    return a * b + (i % 10) * 0.1f;
}

/* Test 1: Target teams distribute parallel for simd with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(64) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(8) safelen(16) aligned(a, b, c: 32) \
            private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        /* Host fallback version */
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 16)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(4) safelen(8) aligned(a, b, c: 64) \
        linear(i:1) private(j) collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c[idx] = a[idx] * b[idx] + (i + j);
        }
    }
}

/* Test 3: Nested SIMD with collapse clause */
void test_nested_simd(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) \
            simdlen(2) safelen(4) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = compute_element(a[idx], b[idx], idx);
            }
        }
    }
}

/* Test 4: Mixed directives - simd inside for */
void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n*M], b[0:n*M]) map(from: c[0:n*M]) \
        if(target: use_gpu_offload)
    for (int i = 0; i < n; i++) {
        #pragma omp simd simdlen(4) aligned(a, b, c: 16) linear(k:1)
        for (int k = 0; k < M; k++) {
            int idx = i * M + k;
            c[idx] = a[idx] * 0.5f + b[idx] * 1.5f;
        }
    }
}

/* Test 5: SIMD with reduction and private clauses */
void test_simd_reduction(float *arr, int n, float *result) {
    float sum = 0.0f;
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) map(to: arr[0:n]) map(from: sum) \
        simdlen(8) if(target: use_gpu_offload)
    for (int i = 0; i < n; i++) {
        sum += arr[i] * arr[i];
    }
    
    *result = sum;
}

/* Initialize arrays with pattern */
void init_arrays(float *a, float *b, int *ia, int *ib, double *da, double *db, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i * 0.1f;
        b[i] = (n - i) * 0.2f;
        ia[i] = i % 100;
        ib[i] = (i * 2) % 100;
        da[i] = i * 0.01;
        db[i] = (n - i) * 0.02;
    }
}

/* Compute checksum to prevent dead code elimination */
float compute_checksum(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
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
    
    /* Allocate arrays with different alignments */
    float *a = (float*)aligned_alloc(32, N * M * sizeof(float));
    float *b = (float*)aligned_alloc(32, N * M * sizeof(float));
    float *c = (float*)aligned_alloc(32, N * M * sizeof(float));
    int *ia = (int*)aligned_alloc(64, N * M * sizeof(int));
    int *ib = (int*)aligned_alloc(64, N * M * sizeof(int));
    int *ic = (int*)aligned_alloc(64, N * M * sizeof(int));
    double *da = (double*)aligned_alloc(32, N * M * sizeof(double));
    double *db = (double*)aligned_alloc(32, N * M * sizeof(double));
    double *dc = (double*)aligned_alloc(32, N * M * sizeof(double));
    
    if (!a || !b || !c || !ia || !ib || !ic || !da || !db || !dc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(a, b, ia, ib, da, db, N * M);
    
    printf("Starting OpenMP SIMD tests...\n");
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("Test 1: Target SIMD\n");
    test_target_simd(a, b, c, N * M);
    float checksum1 = compute_checksum(c, N * M);
    printf("  Checksum: %f\n", checksum1);
    
    /* Test 2: Parallel for SIMD */
    printf("Test 2: Parallel for SIMD\n");
    test_parallel_for_simd(ia, ib, ic, N);
    float checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N * M; i++) {
        checksum2 += ic[i];
    }
    printf("  Checksum: %f\n", checksum2);
    
    /* Test 3: Nested SIMD */
    printf("Test 3: Nested SIMD\n");
    test_nested_simd(da, db, dc, N);
    double checksum3 = 0;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < N * M; i++) {
        checksum3 += dc[i];
    }
    printf("  Checksum: %f\n", (float)checksum3);
    
    /* Test 4: Mixed directives */
    printf("Test 4: Mixed directives\n");
    test_mixed_directives(a, b, c, N);
    float checksum4 = compute_checksum(c, N * M);
    printf("  Checksum: %f\n", checksum4);
    
    /* Test 5: SIMD with reduction */
    printf("Test 5: SIMD reduction\n");
    float reduction_result = 0.0f;
    test_simd_reduction(a, N * M, &reduction_result);
    printf("  Reduction result: %f\n", reduction_result);
    
    /* Validation: Compare with sequential version if GPU was used */
    if (use_gpu_offload) {
        printf("\nValidation: Comparing GPU results with CPU reference...\n");
        
        /* Run CPU-only version for comparison */
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c_cpu = (float*)aligned_alloc(32, N * M * sizeof(float));
        test_target_simd(a, b, c_cpu, N * M);
        
        /* Compare results */
        int errors = 0;
        #pragma omp parallel for simd reduction(+:errors)
        for (int i = 0; i < N * M; i++) {
            if (fabs(c[i] - c_cpu[i]) > 1e-5f) {
                errors++;
            }
        }
        
        if (errors == 0) {
            printf("  Validation PASSED: GPU and CPU results match\n");
        } else {
            printf("  Validation FAILED: %d differences found\n", errors);
        }
        
        free(c_cpu);
        use_gpu_offload = saved_flag;
    }
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(ia); free(ib); free(ic);
    free(da); free(db); free(dc);
    
    printf("\nAll tests completed\n");
    return 0;
}
