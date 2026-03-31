/* test_simt_coverage.c
 * Designed to trigger SIMT transformation in omp-low.cc lines 2941-2975
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower test_simt_coverage.c -o test_simt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64

/* Global flag to control offloading at runtime */
static int use_gpu_offload = 0;

/* Function with declare simd for vectorization */
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
            simdlen(4) safelen(8) aligned(a, b, c: 32) \
            private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        /* Host fallback version */
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32) \
                linear(i:1) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c: 64) \
        schedule(static, 32) private(i) \
        collapse(1) if(n > 512)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

/* Test 3: Nested loops with collapse clause */
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel num_threads(4)
    {
        #pragma omp for simd collapse(2) simdlen(2) \
                aligned(a, b, c: 16) private(i, j) \
                linear(i:1) linear(j:1)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = simd_func(a[idx], b[idx], idx);
            }
        }
    }
}

/* Test 4: Mixed directives - simd inside for */
void test_mixed_directives(double *a, double *b, double *c, int n) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        if(target: use_gpu_offload)
    for (int i = 0; i < n; i++) {
        #pragma omp simd simdlen(2) aligned(a, b, c: 16)
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            c[idx] = a[idx] * 0.5 + b[idx] * 2.0;
        }
    }
}

/* Test 5: Complex reduction with SIMD */
float test_reduction_simd(float *data, int n) {
    float sum = 0.0f;
    
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: data[0:n]) map(tofrom: sum) \
            reduction(+:sum) simdlen(4) num_teams(8)
        for (int i = 0; i < n; i++) {
            sum += data[i] * data[i];
        }
    } else {
        #pragma omp parallel for simd reduction(+:sum) \
                simdlen(8) if(n > 256)
        for (int i = 0; i < n; i++) {
            sum += data[i] * data[i];
        }
    }
    
    return sum;
}

/* Test 6: Dynamic pointer-based access with SIMD */
void test_pointer_simd(float **ptr_arr, int n, int m) {
    #pragma omp target teams distribute parallel for simd \
        map(to: ptr_arr[0:n][0:m]) \
        simdlen(4) collapse(2) private(i, j) \
        if(target: use_gpu_offload)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            ptr_arr[i][j] = ptr_arr[i][j] * 2.0f + (i + j) * 0.1f;
        }
    }
}

/* Initialize arrays with pattern */
void init_arrays(float *a, float *b, int n) {
    #pragma omp parallel for simd if(n > 512)
    for (int i = 0; i < n; i++) {
        a[i] = (float)i;
        b[i] = (float)(n - i);
    }
}

void init_int_arrays(int *a, int *b, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        a[i] = i % 100;
        b[i] = (i * 3) % 100;
    }
}

/* Compute checksum to prevent dead code elimination */
float compute_checksum(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int compute_int_checksum(int *arr, int n) {
    int sum = 0;
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
            printf("GPU offloading enabled via command line\n");
        }
    }
    
    /* Alternatively, check environment variable */
    if (getenv("USE_GPU_OFFLOAD") != NULL) {
        use_gpu_offload = atoi(getenv("USE_GPU_OFFLOAD"));
    }
    
    printf("Running with use_gpu_offload = %d\n", use_gpu_offload);
    
    /* Allocate and initialize arrays */
    float *a = (float*)aligned_alloc(32, N * sizeof(float));
    float *b = (float*)aligned_alloc(32, N * sizeof(float));
    float *c = (float*)aligned_alloc(32, N * sizeof(float));
    float *d = (float*)aligned_alloc(32, N * M * sizeof(float));
    
    int *a_int = (int*)aligned_alloc(64, N * sizeof(int));
    int *b_int = (int*)aligned_alloc(64, N * sizeof(int));
    int *c_int = (int*)aligned_alloc(64, N * sizeof(int));
    
    init_arrays(a, b, N);
    init_int_arrays(a_int, b_int, N);
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("\n=== Test 1: Target SIMD ===\n");
    test_target_simd(a, b, c, N);
    float checksum1 = compute_checksum(c, N);
    printf("Checksum 1: %f\n", checksum1);
    
    /* Test 2: Parallel for SIMD */
    printf("\n=== Test 2: Parallel for SIMD ===\n");
    test_parallel_for_simd(a_int, b_int, c_int, N);
    int checksum2 = compute_int_checksum(c_int, N);
    printf("Checksum 2: %d\n", checksum2);
    
    /* Test 3: Nested SIMD */
    printf("\n=== Test 3: Nested SIMD ===\n");
    test_nested_simd(a, b, d, N, M);
    float checksum3 = compute_checksum(d, N * M);
    printf("Checksum 3: %f\n", checksum3);
    
    /* Test 4: Mixed directives */
    printf("\n=== Test 4: Mixed directives ===\n");
    double *a_dbl = (double*)aligned_alloc(16, N * 4 * sizeof(double));
    double *b_dbl = (double*)aligned_alloc(16, N * 4 * sizeof(double));
    double *c_dbl = (double*)aligned_alloc(16, N * 4 * sizeof(double));
    
    for (int i = 0; i < N * 4; i++) {
        a_dbl[i] = i * 0.5;
        b_dbl[i] = i * 1.5;
    }
    
    test_mixed_directives(a_dbl, b_dbl, c_dbl, N);
    double sum4 = 0.0;
    #pragma omp simd reduction(+:sum4)
    for (int i = 0; i < N * 4; i++) {
        sum4 += c_dbl[i];
    }
    printf("Checksum 4: %lf\n", sum4);
    
    /* Test 5: Reduction with SIMD */
    printf("\n=== Test 5: Reduction SIMD ===\n");
    float reduction_sum = test_reduction_simd(a, N);
    printf("Reduction sum: %f\n", reduction_sum);
    
    /* Test 6: Pointer-based SIMD */
    printf("\n=== Test 6: Pointer SIMD ===\n");
    float **ptr_arr = (float**)malloc(N * sizeof(float*));
    for (int i = 0; i < N; i++) {
        ptr_arr[i] = (float*)aligned_alloc(16, M * sizeof(float));
        for (int j = 0; j < M; j++) {
            ptr_arr[i][j] = (float)(i * M + j);
        }
    }
    
    /* Note: This test requires proper device pointer handling */
    if (use_gpu_offload) {
        printf("Pointer SIMD test requires additional mapping setup\n");
    } else {
        test_pointer_simd(ptr_arr, N, M);
        float ptr_sum = 0.0f;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                ptr_sum += ptr_arr[i][j];
            }
        }
        printf("Pointer array sum: %f\n", ptr_sum);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(a_int); free(b_int); free(c_int);
    free(a_dbl); free(b_dbl); free(c_dbl);
    
    if (ptr_arr) {
        for (int i = 0; i < N; i++) {
            free(ptr_arr[i]);
        }
        free(ptr_arr);
    }
    
    printf("\nAll tests completed successfully!\n");
    
    /* Validate by comparing host-only and GPU results if both were run */
    if (use_gpu_offload) {
        printf("\nNote: Run without --use-gpu to compare host-only results\n");
    }
    
    return 0;
}
