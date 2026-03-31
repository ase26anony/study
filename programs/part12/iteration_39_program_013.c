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

/* Global flag to control offloading */
static int use_gpu_offload = 0;

/* Declare SIMD function */
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_function(float a, float b, int i) {
    return a * b + i * 0.5f;
}

/* Test 1: Target teams distribute parallel for simd with conditional execution */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(128) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) aligned(a, b, c: 32) \
            private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        /* Host fallback version */
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c: 64) \
        schedule(static, 64) private(i) \
        linear(i:1) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

/* Test 3: Nested loops with collapse */
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4) \
            aligned(a, b, c: 32) private(i, j) \
            linear(i:1) linear(j:1)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = simd_function(a[idx], b[idx], idx);
            }
        }
    }
}

/* Test 4: Mixed directives - simd inside for */
void test_mixed_directives(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i += 64) {
            int end = (i + 64) < n ? (i + 64) : n;
            #pragma omp simd simdlen(8) aligned(a, b, c: 64) \
                linear(k:1)
            for (int k = i; k < end; k++) {
                c[k] = a[k] / (b[k] + 1.0);
            }
        }
    }
}

/* Test 5: Dynamic data with pointer accesses */
void test_dynamic_data(int n) {
    float *dyn_a = (float*)malloc(n * sizeof(float));
    float *dyn_b = (float*)malloc(n * sizeof(float));
    float *dyn_c = (float*)malloc(n * sizeof(float));
    
    if (!dyn_a || !dyn_b || !dyn_c) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with pattern */
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < n; i++) {
        dyn_a[i] = i * 0.1f;
        dyn_b[i] = (n - i) * 0.1f;
    }
    
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: dyn_a[0:n], dyn_b[0:n]) map(from: dyn_c[0:n]) \
            simdlen(4) private(i)
        for (int i = 0; i < n; i++) {
            dyn_c[i] = dyn_a[i] * dyn_b[i] + i * 0.01f;
        }
    } else {
        #pragma omp simd simdlen(4)
        for (int i = 0; i < n; i++) {
            dyn_c[i] = dyn_a[i] * dyn_b[i] + i * 0.01f;
        }
    }
    
    /* Compute checksum */
    float checksum = 0.0f;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < n; i++) {
        checksum += dyn_c[i];
    }
    printf("Dynamic data checksum: %.6f\n", checksum);
    
    free(dyn_a);
    free(dyn_b);
    free(dyn_c);
}

/* Validation function */
int validate_results(float *c1, float *c2, int n, float tolerance) {
    int errors = 0;
    #pragma omp parallel for simd reduction(+:errors) simdlen(8)
    for (int i = 0; i < n; i++) {
        if (fabs(c1[i] - c2[i]) > tolerance) {
            errors++;
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
    float *a = (float*)aligned_alloc(32, N * sizeof(float));
    float *b = (float*)aligned_alloc(32, N * sizeof(float));
    float *c1 = (float*)aligned_alloc(32, N * sizeof(float));
    float *c2 = (float*)aligned_alloc(32, N * sizeof(float));
    
    int *ia = (int*)aligned_alloc(64, N * sizeof(int));
    int *ib = (int*)aligned_alloc(64, N * sizeof(int));
    int *ic = (int*)aligned_alloc(64, N * sizeof(int));
    
    double *da = (double*)aligned_alloc(64, N * sizeof(double));
    double *db = (double*)aligned_alloc(64, N * sizeof(double));
    double *dc = (double*)aligned_alloc(64, N * sizeof(double));
    
    float *nd_a = (float*)aligned_alloc(32, N * M * sizeof(float));
    float *nd_b = (float*)aligned_alloc(32, N * M * sizeof(float));
    float *nd_c = (float*)aligned_alloc(32, N * M * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !ia || !ib || !ic || 
        !da || !db || !dc || !nd_a || !nd_b || !nd_c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterns */
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.25f;
        b[i] = (N - i) * 0.25f;
        c1[i] = 0.0f;
        c2[i] = 0.0f;
        
        ia[i] = i;
        ib[i] = N - i;
        ic[i] = 0;
        
        da[i] = i * 0.5;
        db[i] = (N - i) * 0.5;
        dc[i] = 0.0;
    }
    
    #pragma omp parallel for simd collapse(2) simdlen(4)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            nd_a[idx] = (i + j) * 0.1f;
            nd_b[idx] = (N - i + M - j) * 0.1f;
            nd_c[idx] = 0.0f;
        }
    }
    
    printf("Running OpenMP SIMD tests...\n");
    
    /* Run test 1 - Target SIMD with conditional offloading */
    printf("\nTest 1: Target SIMD with conditional offloading\n");
    test_target_simd(a, b, c1, N);
    
    /* Run test 2 - Parallel for SIMD */
    printf("Test 2: Parallel for SIMD\n");
    test_parallel_for_simd(ia, ib, ic, N);
    
    /* Run test 3 - Nested SIMD with collapse */
    printf("Test 3: Nested SIMD with collapse\n");
    test_nested_simd(nd_a, nd_b, nd_c, N, M);
    
    /* Run test 4 - Mixed directives */
    printf("Test 4: Mixed directives\n");
    test_mixed_directives(da, db, dc, N);
    
    /* Run test 5 - Dynamic data */
    printf("Test 5: Dynamic data with pointer accesses\n");
    test_dynamic_data(N);
    
    /* Compute checksums to prevent dead code elimination */
    float checksum1 = 0.0f, checksum2 = 0.0f;
    int checksum_int = 0;
    double checksum_double = 0.0;
    float checksum_nd = 0.0f;
    
    #pragma omp simd reduction(+:checksum1) simdlen(8)
    for (int i = 0; i < N; i++) {
        checksum1 += c1[i];
    }
    
    #pragma omp simd reduction(+:checksum_int) simdlen(8)
    for (int i = 0; i < N; i++) {
        checksum_int += ic[i];
    }
    
    #pragma omp simd reduction(+:checksum_double) simdlen(8)
    for (int i = 0; i < N; i++) {
        checksum_double += dc[i];
    }
    
    #pragma omp simd reduction(+:checksum_nd) simdlen(4)
    for (int i = 0; i < N * M; i++) {
        checksum_nd += nd_c[i];
    }
    
    printf("\nChecksums (for validation):\n");
    printf("  Test 1: %.6f\n", checksum1);
    printf("  Test 2: %d\n", checksum_int);
    printf("  Test 3: %.6f\n", checksum_nd);
    printf("  Test 4: %.6f\n", checksum_double);
    
    /* If GPU offloading was enabled, run host version and compare */
    if (use_gpu_offload) {
        printf("\nRunning host-only version for comparison...\n");
        int old_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        /* Re-initialize output arrays */
        #pragma omp simd simdlen(8)
        for (int i = 0; i < N; i++) {
            c2[i] = 0.0f;
        }
        
        test_target_simd(a, b, c2, N);
        
        /* Validate results */
        int errors = validate_results(c1, c2, N, 0.001f);
        if (errors == 0) {
            printf("Validation PASSED: GPU and CPU results match\n");
        } else {
            printf("Validation FAILED: %d differences found\n", errors);
        }
        
        use_gpu_offload = old_flag;
    }
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2);
    free(ia); free(ib); free(ic);
    free(da); free(db); free(dc);
    free(nd_a); free(nd_b); free(nd_c);
    
    printf("\nAll tests completed.\n");
    return 0;
}
