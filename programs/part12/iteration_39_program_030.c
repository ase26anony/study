/* test_simt_transformation.c
 * This program is designed to trigger the SIMT transformation logic
 * in GCC's omp-low.cc (lines 2941-2975) for OpenMP GPU offloading.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32
#define CHUNK_SIZE 64

/* Global flag to control GPU offloading */
static int use_gpu_offload = 0;

/* Function declared with SIMD attribute */
#pragma omp declare simd uniform(b) linear(i:1) notinbranch
float simd_mul(float a, float b, int i) {
    return a * b + i * 0.1f;
}

/* Test 1: Target teams distribute parallel for simd with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        num_teams(4) thread_limit(128) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(4) safelen(8) aligned(a, b, c: 32) \
        private(n) reduction(+:n)
    for (int i = 0; i < n; i++) {
        float temp = a[i] + b[i];
        c[i] = temp * 0.5f + i * 0.01f;
        n += i;  // dummy reduction
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c: 64) \
        schedule(static, CHUNK_SIZE) \
        linear(i:1) lastprivate(i)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

/* Test 3: Nested SIMD with collapse */
void test_nested_simd(double *a, double *b, double *c, int n, int m) {
    #pragma omp parallel num_threads(4)
    {
        #pragma omp for simd collapse(2) \
            simdlen(2) safelen(4) \
            private(i, j) \
            reduction(max: max_val)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * b[idx] + simd_mul(a[idx], b[idx], idx);
            }
        }
    }
}

/* Test 4: SIMD with if clause and multiple data types */
void test_mixed_simd(float *fa, double *da, float *fc, double *dc, int n) {
    /* First SIMD loop with if clause */
    #pragma omp simd simdlen(4) if(simd: n > 100) aligned(fa, fc: 16)
    for (int i = 0; i < n; i++) {
        fc[i] = fa[i] * 2.0f + i * 0.1f;
    }
    
    /* Second SIMD loop with different characteristics */
    #pragma omp simd simdlen(8) safelen(32) linear(i:2)
    for (int i = 0; i < n; i += 2) {
        dc[i/2] = da[i] * 3.0 + i * 0.01;
    }
}

/* Test 5: Complex target region with multiple SIMD constructs */
void test_complex_target(int *a, int *b, int *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(tofrom: c[0:n]) \
            num_teams(8) num_threads(64) \
            simdlen(4) collapse(2) \
            private(i, j) firstprivate(n)
        for (int i = 0; i < n/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                c[idx] = a[idx] + b[idx] * (i + j);
            }
        }
    } else {
        #pragma omp parallel for simd simdlen(8)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

/* Initialize arrays with patterned data */
void init_arrays(float *fa, double *da, int *ia, int n) {
    #pragma omp parallel for simd simdlen(8) if(n > 256)
    for (int i = 0; i < n; i++) {
        fa[i] = i * 1.5f;
        da[i] = i * 2.5;
        ia[i] = i * 3;
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(float *farr, double *darr, int *iarr, int n) {
    double sum = 0.0;
    
    #pragma omp simd reduction(+:sum) simdlen(4)
    for (int i = 0; i < n; i++) {
        sum += farr[i] + darr[i] + iarr[i];
    }
    
    return sum;
}

/* Validate results between host and potential device execution */
int validate_results(float *c1, float *c2, int n, float tolerance) {
    int errors = 0;
    
    #pragma omp simd simdlen(8) reduction(+:errors)
    for (int i = 0; i < n; i++) {
        if (fabs(c1[i] - c2[i]) > tolerance) {
            errors++;
        }
    }
    
    return errors;
}

int main(int argc, char *argv[]) {
    /* Parse command line argument for GPU offloading */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    /* Set environment variable to influence runtime behavior */
    if (use_gpu_offload) {
        setenv("OMP_TARGET_OFFLOAD", "MANDATORY", 1);
    }
    
    /* Allocate arrays with different alignments */
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc1 = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc2 = (float*)aligned_alloc(32, N * sizeof(float));
    
    double *da = (double*)aligned_alloc(64, N * sizeof(double));
    double *db = (double*)aligned_alloc(64, N * sizeof(double));
    double *dc = (double*)aligned_alloc(64, N * sizeof(double));
    
    int *ia = (int*)aligned_alloc(64, N * sizeof(int));
    int *ib = (int*)aligned_alloc(64, N * sizeof(int));
    int *ic = (int*)aligned_alloc(64, N * sizeof(int));
    
    /* Initialize arrays */
    init_arrays(fa, da, ia, N);
    init_arrays(fb, db, ib, N);
    
    /* Clear output arrays */
    memset(fc1, 0, N * sizeof(float));
    memset(fc2, 0, N * sizeof(float));
    memset(dc, 0, N * sizeof(double));
    memset(ic, 0, N * sizeof(int));
    
    printf("Starting OpenMP SIMD tests...\n");
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("\nTest 1: Target teams distribute parallel for simd\n");
    test_target_simd(fa, fb, fc1, N);
    double checksum1 = compute_checksum(fc1, dc, ic, N);
    printf("Checksum 1: %f\n", checksum1);
    
    /* Test 2: Parallel for simd */
    printf("\nTest 2: Parallel for simd\n");
    test_parallel_for_simd(ia, ib, ic, N);
    double checksum2 = compute_checksum(fc1, dc, ic, N);
    printf("Checksum 2: %f\n", checksum2);
    
    /* Test 3: Nested SIMD */
    printf("\nTest 3: Nested SIMD with collapse\n");
    test_nested_simd(da, db, dc, M, M);
    double checksum3 = compute_checksum(fc1, dc, ic, N);
    printf("Checksum 3: %f\n", checksum3);
    
    /* Test 4: Mixed SIMD */
    printf("\nTest 4: Mixed SIMD with different data types\n");
    test_mixed_simd(fa, da, fc2, dc, N);
    double checksum4 = compute_checksum(fc2, dc, ic, N);
    printf("Checksum 4: %f\n", checksum4);
    
    /* Test 5: Complex target region */
    printf("\nTest 5: Complex target region\n");
    test_complex_target(ia, ib, ic, N);
    double checksum5 = compute_checksum(fc1, dc, ic, N);
    printf("Checksum 5: %f\n", checksum5);
    
    /* Validation if we ran both paths */
    if (use_gpu_offload) {
        /* Run host version for comparison */
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *fc_host = (float*)aligned_alloc(32, N * sizeof(float));
        memset(fc_host, 0, N * sizeof(float));
        
        test_target_simd(fa, fb, fc_host, N);
        
        int errors = validate_results(fc1, fc_host, N, 0.001f);
        printf("\nValidation: %d errors between GPU and host execution\n", errors);
        
        free(fc_host);
        use_gpu_offload = saved_flag;
    }
    
    /* Free allocated memory */
    free(fa); free(fb); free(fc1); free(fc2);
    free(da); free(db); free(dc);
    free(ia); free(ib); free(ic);
    
    printf("\nAll tests completed.\n");
    return 0;
}
