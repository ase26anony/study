#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

static int use_gpu_offload = 0;

/* Declare SIMD function */
#pragma omp declare simd uniform(a, b) linear(i:1) simdlen(8)
float simd_function(float a, float b, int i) {
    return a * b + i * 0.5f;
}

/* Test 1: Target teams distribute parallel for simd with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        /* Host fallback - still SIMD but no offloading */
        #pragma omp simd simdlen(4) aligned(a, b, c: 32) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(8) collapse(2) \
        aligned(a, b, c: 64) private(i, j) schedule(static)
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
        #pragma omp for simd collapse(2) simdlen(4) safelen(16) \
            private(i, j) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int idx = i * n + j;
                c[idx] = a[idx] * 0.5 + b[idx] * 2.0;
            }
        }
    }
}

/* Test 4: SIMD function calls within target region */
void test_simd_function_calls(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(8) private(i)
        for (int i = 0; i < n; i++) {
            c[i] = simd_function(a[i], b[i], i);
        }
    } else {
        #pragma omp simd simdlen(8)
        for (int i = 0; i < n; i++) {
            c[i] = simd_function(a[i], b[i], i);
        }
    }
}

/* Test 5: Mixed directives - SIMD inside for */
void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for private(i) nowait
        for (int i = 0; i < n; i++) {
            #pragma omp simd simdlen(4) aligned(a, b, c: 16)
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx];
            }
        }
    }
}

/* Validation function */
int validate_results(float *c1, float *c2, int n, const char *test_name) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (c1[i] != c2[i]) {
            errors++;
            if (errors < 5) {
                printf("  Mismatch at %d: %f != %f\n", i, c1[i], c2[i]);
            }
        }
    }
    if (errors > 0) {
        printf("%s: %d errors\n", test_name, errors);
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
    float *a_f = (float *)aligned_alloc(64, N * M * sizeof(float));
    float *b_f = (float *)aligned_alloc(64, N * M * sizeof(float));
    float *c1_f = (float *)aligned_alloc(64, N * M * sizeof(float));
    float *c2_f = (float *)aligned_alloc(64, N * M * sizeof(float));
    
    int *a_i = (int *)aligned_alloc(64, N * M * sizeof(int));
    int *b_i = (int *)aligned_alloc(64, N * M * sizeof(int));
    int *c_i = (int *)aligned_alloc(64, N * M * sizeof(int));
    
    double *a_d = (double *)aligned_alloc(64, N * N * sizeof(double));
    double *b_d = (double *)aligned_alloc(64, N * N * sizeof(double));
    double *c_d = (double *)aligned_alloc(64, N * N * sizeof(double));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N * M; i++) {
        a_f[i] = i * 0.1f;
        b_f[i] = (N * M - i) * 0.2f;
        a_i[i] = i;
        b_i[i] = (N * M - i) * 2;
        c1_f[i] = 0.0f;
        c2_f[i] = 0.0f;
        c_i[i] = 0;
    }
    
    for (int i = 0; i < N * N; i++) {
        a_d[i] = i * 0.01;
        b_d[i] = (N * N - i) * 0.02;
        c_d[i] = 0.0;
    }
    
    printf("Running OpenMP SIMD tests...\n");
    
    /* Run tests with GPU offloading if enabled */
    int original_use_gpu = use_gpu_offload;
    
    /* Test 1: Target SIMD */
    printf("Test 1: Target teams distribute parallel for simd\n");
    test_target_simd(a_f, b_f, c1_f, N * M);
    
    /* Compute checksum to prevent dead code elimination */
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N * M; i++) {
        checksum1 += c1_f[i];
    }
    printf("  Checksum: %f\n", checksum1);
    
    /* Test 2: Parallel for SIMD */
    printf("Test 2: Parallel for simd with collapse\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    int checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N * M; i++) {
        checksum2 += c_i[i];
    }
    printf("  Checksum: %d\n", checksum2);
    
    /* Test 3: Nested SIMD */
    printf("Test 3: Nested SIMD with collapse\n");
    test_nested_simd(a_d, b_d, c_d, N);
    
    double checksum3 = 0.0;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < N * N; i++) {
        checksum3 += c_d[i];
    }
    printf("  Checksum: %f\n", checksum3);
    
    /* Test 4: SIMD function calls */
    printf("Test 4: SIMD function calls in target region\n");
    test_simd_function_calls(a_f, b_f, c2_f, N * M);
    
    float checksum4 = 0.0f;
    #pragma omp simd reduction(+:checksum4)
    for (int i = 0; i < N * M; i++) {
        checksum4 += c2_f[i];
    }
    printf("  Checksum: %f\n", checksum4);
    
    /* Test 5: Mixed directives */
    printf("Test 5: Mixed directives (for with inner simd)\n");
    test_mixed_directives(a_f, b_f, c1_f, N);
    
    /* If GPU was enabled, also run host-only version for comparison */
    if (original_use_gpu) {
        printf("\nRunning host-only comparison...\n");
        use_gpu_offload = 0;
        
        float *c_host = (float *)aligned_alloc(64, N * M * sizeof(float));
        memset(c_host, 0, N * M * sizeof(float));
        
        test_target_simd(a_f, b_f, c_host, N * M);
        
        /* Validate GPU vs CPU results */
        int errors = validate_results(c1_f, c_host, N * M, "Target SIMD validation");
        if (errors == 0) {
            printf("  GPU and CPU results match!\n");
        }
        
        free(c_host);
    }
    
    /* Cleanup */
    free(a_f);
    free(b_f);
    free(c1_f);
    free(c2_f);
    free(a_i);
    free(b_i);
    free(c_i);
    free(a_d);
    free(b_d);
    free(c_d);
    
    printf("\nAll tests completed.\n");
    return 0;
}
