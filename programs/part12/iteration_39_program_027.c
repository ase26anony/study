#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

int use_gpu_offload = 0;

#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_function(float a, float b, int i) {
    return a * b + i * 0.5f;
}

void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) num_teams(4) thread_limit(128)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(8) safelen(16) \
        private(a, b, c) aligned(a, b, c: 64) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for collapse(2) simd simdlen(4)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = simd_function(a[idx], b[idx], idx);
            }
        }
    }
}

void test_reduction_simd(float *arr, int n, float *result) {
    float sum = 0.0f;
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) map(to: arr[0:n]) map(from: sum) \
        simdlen(4) num_teams(8)
    for (int i = 0; i < n; i++) {
        sum += arr[i] * arr[i];
    }
    
    *result = sum;
}

void test_mixed_directives(double *a, double *b, double *c, int n) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        #pragma omp simd simdlen(4) aligned(a, b, c: 32)
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx < n * 4) {
                c[idx] = a[idx] + b[idx];
            }
        }
    }
}

float compute_checksum(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    // Parse command line argument
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
        }
    }
    
    // Set environment variable to influence runtime behavior
    if (use_gpu_offload) {
        setenv("OMP_TARGET_OFFLOAD", "MANDATORY", 1);
    }
    
    printf("Using GPU offload: %s\n", use_gpu_offload ? "YES" : "NO");
    
    // Allocate and initialize arrays
    float *a_f = (float *)aligned_alloc(64, N * M * sizeof(float));
    float *b_f = (float *)aligned_alloc(64, N * M * sizeof(float));
    float *c_f = (float *)aligned_alloc(64, N * M * sizeof(float));
    
    int *a_i = (int *)aligned_alloc(64, N * sizeof(int));
    int *b_i = (int *)aligned_alloc(64, N * sizeof(int));
    int *c_i = (int *)aligned_alloc(64, N * sizeof(int));
    
    double *a_d = (double *)aligned_alloc(64, N * 4 * sizeof(double));
    double *b_d = (double *)aligned_alloc(64, N * 4 * sizeof(double));
    double *c_d = (double *)aligned_alloc(64, N * 4 * sizeof(double));
    
    // Initialize with patterned data
    #pragma omp parallel for simd
    for (int i = 0; i < N * M; i++) {
        a_f[i] = i * 0.1f;
        b_f[i] = (N * M - i) * 0.2f;
        c_f[i] = 0.0f;
    }
    
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        a_i[i] = i;
        b_i[i] = N - i;
        c_i[i] = 0;
    }
    
    #pragma omp simd
    for (int i = 0; i < N * 4; i++) {
        a_d[i] = i * 0.01;
        b_d[i] = i * 0.02;
        c_d[i] = 0.0;
    }
    
    // Run test functions
    printf("Test 1: Target SIMD with conditional offloading\n");
    test_target_simd(a_f, b_f, c_f, N * M);
    float checksum1 = compute_checksum(c_f, N * M);
    printf("Checksum 1: %f\n", checksum1);
    
    printf("\nTest 2: Parallel for SIMD (host-only)\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    int checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N; i++) {
        checksum2 += c_i[i];
    }
    printf("Checksum 2: %d\n", checksum2);
    
    printf("\nTest 3: Nested SIMD with collapse\n");
    test_nested_simd(a_f, b_f, c_f, N, M);
    float checksum3 = compute_checksum(c_f, N * M);
    printf("Checksum 3: %f\n", checksum3);
    
    printf("\nTest 4: Reduction in SIMD context\n");
    float reduction_result = 0.0f;
    test_reduction_simd(a_f, N * M, &reduction_result);
    printf("Reduction result: %f\n", reduction_result);
    
    printf("\nTest 5: Mixed directives (for + simd)\n");
    test_mixed_directives(a_d, b_d, c_d, N);
    double checksum5 = 0.0;
    #pragma omp simd reduction(+:checksum5)
    for (int i = 0; i < N * 4; i++) {
        checksum5 += c_d[i];
    }
    printf("Checksum 5: %lf\n", checksum5);
    
    // Validation step - compare with reference implementation
    printf("\nValidation:\n");
    float *ref_c = (float *)aligned_alloc(64, N * M * sizeof(float));
    
    // Reference computation (sequential)
    for (int i = 0; i < N * M; i++) {
        ref_c[i] = a_f[i] + b_f[i] * 2.0f;
    }
    
    float ref_checksum = 0.0f;
    for (int i = 0; i < N * M; i++) {
        ref_checksum += ref_c[i];
    }
    
    // Recompute with SIMD for comparison
    #pragma omp simd simdlen(4)
    for (int i = 0; i < N * M; i++) {
        c_f[i] = a_f[i] + b_f[i] * 2.0f;
    }
    float simd_checksum = compute_checksum(c_f, N * M);
    
    printf("Reference checksum: %f\n", ref_checksum);
    printf("SIMD checksum: %f\n", simd_checksum);
    printf("Difference: %e\n", ref_checksum - simd_checksum);
    
    // Cleanup
    free(a_f);
    free(b_f);
    free(c_f);
    free(a_i);
    free(b_i);
    free(c_i);
    free(a_d);
    free(b_d);
    free(c_d);
    free(ref_c);
    
    return 0;
}
