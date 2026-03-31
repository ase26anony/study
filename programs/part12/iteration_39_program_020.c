/* test_simt_transformation.c
 * Designed to trigger GCC's SIMT transformation in omp-low.cc lines 2941-2975
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64
#define CHUNK_SIZE 128

/* Global flag to control offloading at runtime */
static int use_gpu_offload = 0;

/* Function declared with SIMD attribute */
#pragma omp declare simd uniform(b) linear(i:1)
float simd_function(float a, float b, int i) {
    return a * b + (i % 10) * 0.1f;
}

/* Test 1: Target teams distribute parallel for simd with conditional execution */
void test_target_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
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
        linear(i:1) \
        schedule(static, CHUNK_SIZE)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

/* Test 3: Nested loops with collapse clause */
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) \
            simdlen(4) \
            private(i, j) \
            reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * b[idx] + simd_function(a[idx], b[idx], idx);
            }
        }
    }
}

/* Test 4: Teams distribute with nested parallel for simd */
void test_teams_distribute_simd(double *a, double *b, double *c, int n) {
    #pragma omp target teams distribute \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; i += CHUNK_SIZE) {
        #pragma omp parallel for simd \
            simdlen(2) \
            private(j) \
            lastprivate(last_val)
        for (int j = i; j < i + CHUNK_SIZE && j < n; j++) {
            c[j] = a[j] / (b[j] + 1.0);
            last_val = c[j];
        }
    }
}

/* Test 5: SIMD with reduction and multiple data types */
void test_mixed_reduction(float *arr_f, int *arr_i, double *arr_d, int n) {
    float sum_f = 0.0f;
    int sum_i = 0;
    double sum_d = 0.0;
    
    #pragma omp parallel for simd \
        reduction(+:sum_f, sum_i, sum_d) \
        simdlen(4) \
        private(i) \
        collapse(1)
    for (int i = 0; i < n; i++) {
        sum_f += arr_f[i];
        sum_i += arr_i[i];
        sum_d += arr_d[i];
    }
    
    printf("Reductions: float=%f, int=%d, double=%lf\n", sum_f, sum_i, sum_d);
}

/* Test 6: Dynamic pointer-based access with SIMD */
void test_pointer_simd(float **ptr_arr, int n, int m) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: ptr_arr[0:n][0:m]) \
        simdlen(4) \
        is_device_ptr(ptr_arr)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            ptr_arr[i][j] = ptr_arr[i][j] * 1.5f + (i + j) * 0.01f;
        }
    }
}

/* Validation function */
int validate_results(float *c1, float *c2, int n, const char *test_name) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (fabs(c1[i] - c2[i]) > 1e-6) {
            errors++;
            if (errors < 5) {
                printf("  Mismatch at %d: %f != %f\n", i, c1[i], c2[i]);
            }
        }
    }
    if (errors > 0) {
        printf("%s: %d errors found\n", test_name, errors);
    }
    return errors;
}

int main(int argc, char **argv) {
    /* Parse command line argument for GPU offloading */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled via command line\n");
        }
    }
    
    /* Also check environment variable */
    if (getenv("USE_GPU_OFFLOAD") != NULL) {
        use_gpu_offload = atoi(getenv("USE_GPU_OFFLOAD"));
    }
    
    printf("Running with use_gpu_offload = %d\n", use_gpu_offload);
    
    /* Allocate and initialize test data */
    float *a_f = (float*)malloc(N * sizeof(float));
    float *b_f = (float*)malloc(N * sizeof(float));
    float *c1_f = (float*)malloc(N * sizeof(float));
    float *c2_f = (float*)malloc(N * sizeof(float));
    
    int *a_i = (int*)malloc(N * sizeof(int));
    int *b_i = (int*)malloc(N * sizeof(int));
    int *c_i = (int*)malloc(N * sizeof(int));
    
    double *a_d = (double*)malloc(N * sizeof(double));
    double *b_d = (double*)malloc(N * sizeof(double));
    double *c_d = (double*)malloc(N * sizeof(double));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 0.1f;
        b_f[i] = (N - i) * 0.2f;
        c1_f[i] = 0.0f;
        c2_f[i] = 0.0f;
        
        a_i[i] = i;
        b_i[i] = N - i;
        c_i[i] = 0;
        
        a_d[i] = i * 0.01;
        b_d[i] = (N - i) * 0.02;
        c_d[i] = 0.0;
    }
    
    printf("\n=== Test 1: Target SIMD with conditional offloading ===\n");
    test_target_simd(a_f, b_f, c1_f, N);
    
    /* Compute checksum to prevent dead code elimination */
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N; i++) {
        checksum1 += c1_f[i];
    }
    printf("Checksum 1: %f\n", checksum1);
    
    printf("\n=== Test 2: Parallel for SIMD ===\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    int checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N; i++) {
        checksum2 += c_i[i];
    }
    printf("Checksum 2: %d\n", checksum2);
    
    printf("\n=== Test 3: Nested SIMD with collapse ===\n");
    float *a_2d = (float*)malloc(N * M * sizeof(float));
    float *b_2d = (float*)malloc(N * M * sizeof(float));
    float *c_2d = (float*)malloc(N * M * sizeof(float));
    
    for (int i = 0; i < N * M; i++) {
        a_2d[i] = (i % 100) * 0.1f;
        b_2d[i] = (i % 50) * 0.2f;
        c_2d[i] = 0.0f;
    }
    
    test_nested_simd(a_2d, b_2d, c_2d, N, M);
    
    float checksum3 = 0.0f;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < N * M; i++) {
        checksum3 += c_2d[i];
    }
    printf("Checksum 3: %f\n", checksum3);
    
    printf("\n=== Test 4: Teams distribute with nested SIMD ===\n");
    test_teams_distribute_simd(a_d, b_d, c_d, N);
    
    double checksum4 = 0.0;
    #pragma omp simd reduction(+:checksum4)
    for (int i = 0; i < N; i++) {
        checksum4 += c_d[i];
    }
    printf("Checksum 4: %lf\n", checksum4);
    
    printf("\n=== Test 5: Mixed reduction SIMD ===\n");
    test_mixed_reduction(a_f, a_i, a_d, N);
    
    printf("\n=== Test 6: Pointer-based SIMD ===\n");
    float **ptr_arr = (float**)malloc(16 * sizeof(float*));
    for (int i = 0; i < 16; i++) {
        ptr_arr[i] = (float*)malloc(16 * sizeof(float));
        for (int j = 0; j < 16; j++) {
            ptr_arr[i][j] = (i * 16 + j) * 0.1f;
        }
    }
    
    test_pointer_simd(ptr_arr, 16, 16);
    
    float checksum6 = 0.0f;
    for (int i = 0; i < 16; i++) {
        #pragma omp simd reduction(+:checksum6)
        for (int j = 0; j < 16; j++) {
            checksum6 += ptr_arr[i][j];
        }
    }
    printf("Checksum 6: %f\n", checksum6);
    
    /* Run GPU version if not already done */
    if (!use_gpu_offload) {
        printf("\n=== Running GPU version for comparison ===\n");
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 1;
        
        /* Re-initialize output arrays */
        memset(c2_f, 0, N * sizeof(float));
        
        test_target_simd(a_f, b_f, c2_f, N);
        
        /* Validate GPU vs CPU results */
        validate_results(c1_f, c2_f, N, "GPU vs CPU validation");
        
        use_gpu_offload = saved_flag;
    }
    
    /* Cleanup */
    free(a_f); free(b_f); free(c1_f); free(c2_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    free(a_2d); free(b_2d); free(c_2d);
    
    for (int i = 0; i < 16; i++) {
        free(ptr_arr[i]);
    }
    free(ptr_arr);
    
    printf("\nAll tests completed.\n");
    return 0;
}
