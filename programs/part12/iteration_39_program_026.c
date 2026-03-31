/* test_simt_coverage.c
 * Designed to trigger SIMT transformation in omp-low.cc lines 2941-2975
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower test_simt_coverage.c -o test_simt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

/* Global flag to control offloading at runtime */
static int use_gpu_offload = 0;

/* Function with declare simd for vectorization */
#pragma omp declare simd uniform(a, b) linear(i:1) simdlen(4)
float simd_function(float a, float b, int i) {
    return a * b + i * 0.5f;
}

/* Test 1: Target teams distribute parallel for simd with conditional execution */
void test_target_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(8) safelen(16) aligned(a, b, c: 32) \
        private(n) num_teams(4) thread_limit(128)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(4) safelen(8) aligned(a, b, c: 16) \
        linear(i:1) reduction(+:n)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
        n += i;  /* dummy reduction operation */
    }
}

/* Test 3: Nested loops with collapse clause */
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) private(i, j) \
            simdlen(4) ordered
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] - b[idx];
                /* Call declare simd function */
                c[idx] += simd_function(a[idx], b[idx], idx);
            }
        }
    }
}

/* Test 4: SIMD with if clause and dynamic condition */
void test_conditional_simd(double *a, double *b, double *c, int n, int flag) {
    #pragma omp simd if(flag > 0) simdlen(4) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

/* Test 5: Complex reduction with SIMD */
float test_reduction_simd(float *data, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) simdlen(8) safelen(32)
    for (int i = 0; i < n; i++) {
        sum += data[i] * data[i];
    }
    
    return sum;
}

/* Test 6: Multiple SIMD regions with pointer aliasing */
void test_pointer_simd(float **ptr_arr, int n) {
    float *local_ptr = (float*)malloc(n * sizeof(float));
    
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: n) map(tofrom: local_ptr[0:n]) \
        map(to: ptr_arr[0][0:n]) \
        simdlen(4) num_teams(8)
    for (int i = 0; i < n; i++) {
        local_ptr[i] = ptr_arr[0][i] + ptr_arr[1][i];
    }
    
    free(local_ptr);
}

/* Helper function to compute checksum */
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
            printf("GPU offloading enabled via runtime flag\n");
        }
    }
    
    /* Allocate and initialize test data */
    float *a = (float*)aligned_alloc(32, N * sizeof(float));
    float *b = (float*)aligned_alloc(32, N * sizeof(float));
    float *c = (float*)aligned_alloc(32, N * sizeof(float));
    float *d = (float*)aligned_alloc(32, N * M * sizeof(float));
    float *e = (float*)aligned_alloc(32, N * M * sizeof(float));
    float *f = (float*)aligned_alloc(32, N * M * sizeof(float));
    
    int *ia = (int*)aligned_alloc(16, N * sizeof(int));
    int *ib = (int*)aligned_alloc(16, N * sizeof(int));
    int *ic = (int*)aligned_alloc(16, N * sizeof(int));
    
    /* Initialize with patterned data */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0f;
        b[i] = N - i * 1.0f;
        ia[i] = i;
        ib[i] = i * 2;
    }
    
    for (int i = 0; i < N * M; i++) {
        d[i] = i * 0.5f;
        e[i] = i * 1.5f;
    }
    
    /* Create pointer array for aliasing test */
    float **ptr_arr = (float**)malloc(2 * sizeof(float*));
    ptr_arr[0] = a;
    ptr_arr[1] = b;
    
    printf("Starting OpenMP SIMD tests with use_gpu_offload = %d\n", use_gpu_offload);
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("\nTest 1: Target teams distribute parallel for simd\n");
    test_target_simd(a, b, c, N);
    float checksum1 = compute_checksum(c, N);
    printf("Checksum 1: %f\n", checksum1);
    
    /* Test 2: Parallel for SIMD */
    printf("\nTest 2: Parallel for simd\n");
    test_parallel_for_simd(ia, ib, ic, N);
    int sum2 = 0;
    #pragma omp simd reduction(+:sum2)
    for (int i = 0; i < N; i++) sum2 += ic[i];
    printf("Sum 2: %d\n", sum2);
    
    /* Test 3: Nested SIMD */
    printf("\nTest 3: Nested simd with collapse\n");
    test_nested_simd(d, e, f, N, M);
    float checksum3 = compute_checksum(f, N * M);
    printf("Checksum 3: %f\n", checksum3);
    
    /* Test 4: Conditional SIMD */
    printf("\nTest 4: Conditional simd\n");
    double *da = (double*)aligned_alloc(64, N * sizeof(double));
    double *db = (double*)aligned_alloc(64, N * sizeof(double));
    double *dc = (double*)aligned_alloc(64, N * sizeof(double));
    
    for (int i = 0; i < N; i++) {
        da[i] = i * 0.25;
        db[i] = i * 0.75;
    }
    
    test_conditional_simd(da, db, dc, N, use_gpu_offload);
    double sum4 = 0.0;
    #pragma omp simd reduction(+:sum4)
    for (int i = 0; i < N; i++) sum4 += dc[i];
    printf("Sum 4: %lf\n", sum4);
    
    /* Test 5: Reduction SIMD */
    printf("\nTest 5: Reduction simd\n");
    float reduction_result = test_reduction_simd(a, N);
    printf("Reduction result: %f\n", reduction_result);
    
    /* Test 6: Pointer aliasing SIMD */
    printf("\nTest 6: Pointer aliasing simd\n");
    test_pointer_simd(ptr_arr, N);
    
    /* Validation: Compare with host-only computation if GPU was used */
    if (use_gpu_offload) {
        printf("\nValidating GPU results against host computation...\n");
        float *c_host = (float*)malloc(N * sizeof(float));
        
        /* Host-only computation */
        int save_flag = use_gpu_offload;
        use_gpu_offload = 0;
        test_target_simd(a, b, c_host, N);
        use_gpu_offload = save_flag;
        
        /* Compare */
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (fabs(c[i] - c_host[i]) > 1e-5) {
                errors++;
                if (errors < 5) {
                    printf("Mismatch at %d: GPU=%f, Host=%f\n", i, c[i], c_host[i]);
                }
            }
        }
        printf("Validation: %d errors out of %d elements\n", errors, N);
        free(c_host);
    }
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(d); free(e); free(f);
    free(ia); free(ib); free(ic);
    free(da); free(db); free(dc);
    free(ptr_arr);
    
    printf("\nAll tests completed.\n");
    return 0;
}
