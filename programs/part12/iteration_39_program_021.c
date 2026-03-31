#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

/* Global flag to control GPU offloading */
static int use_gpu_offload = 0;

/* Function declared with SIMD attribute */
#pragma omp declare simd uniform(b) linear(i:1)
float simd_multiply(float a, float b, int i) {
    return a * b * (i % 10 + 1);
}

/* Test 1: Target teams distribute parallel for simd with conditional execution */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(64) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(8) safelen(16) aligned(a, b, c: 32) \
            private(n) reduction(+:n)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_multiply(a[i], b[i], i);
        }
    } else {
        /* Host fallback version */
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 16) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(4) safelen(8) collapse(2) \
        aligned(a, b, c: 16) private(n) \
        schedule(static, 16)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c[idx] = a[idx] * 2 + b[idx] * 3;
        }
    }
}

/* Test 3: Nested SIMD with collapse */
void test_nested_simd(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) \
            simdlen(2) safelen(4) \
            aligned(a, b, c: 64) \
            linear(i:1) linear(j:1)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] * b[idx] / (i + j + 1.0);
            }
        }
    }
}

/* Test 4: Mixed directives - SIMD inside parallel region */
void test_mixed_simd(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            #pragma omp simd simdlen(8) aligned(a, b, c: 32)
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = (a[idx] + b[idx]) * 0.5f;
            }
        }
    }
}

/* Test 5: Target simd with dynamic data */
void test_dynamic_simd(int size) {
    int *dyn_a = (int *)malloc(size * sizeof(int));
    int *dyn_b = (int *)malloc(size * sizeof(int));
    int *dyn_c = (int *)malloc(size * sizeof(int));
    
    if (!dyn_a || !dyn_b || !dyn_c) {
        fprintf(stderr, "Memory allocation failed\n");
        free(dyn_a); free(dyn_b); free(dyn_c);
        return;
    }
    
    /* Initialize with pattern */
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < size; i++) {
        dyn_a[i] = i % 100;
        dyn_b[i] = (size - i) % 100;
    }
    
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: dyn_a[0:size], dyn_b[0:size]) \
            map(from: dyn_c[0:size]) \
            simdlen(8) safelen(16)
        for (int i = 0; i < size; i++) {
            dyn_c[i] = dyn_a[i] + dyn_b[i] * 2;
        }
    } else {
        #pragma omp simd simdlen(4)
        for (int i = 0; i < size; i++) {
            dyn_c[i] = dyn_a[i] + dyn_b[i];
        }
    }
    
    /* Compute checksum */
    long long checksum = 0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < size; i++) {
        checksum += dyn_c[i];
    }
    printf("Dynamic SIMD checksum: %lld\n", checksum);
    
    free(dyn_a); free(dyn_b); free(dyn_c);
}

/* Compute checksum for validation */
long long compute_checksum(float *arr, int n) {
    long long sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += (long long)(arr[i] * 1000);
    }
    return sum;
}

long long compute_checksum_int(int *arr, int n) {
    long long sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

long long compute_checksum_double(double *arr, int n) {
    long long sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += (long long)(arr[i] * 1000);
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
    
    /* Set environment variable for runtime check */
    if (use_gpu_offload) {
        setenv("OMP_TARGET_OFFLOAD", "MANDATORY", 1);
    }
    
    const int total_size = N * M;
    
    /* Allocate and initialize arrays */
    float *a_f = (float *)aligned_alloc(32, total_size * sizeof(float));
    float *b_f = (float *)aligned_alloc(32, total_size * sizeof(float));
    float *c_f = (float *)aligned_alloc(32, total_size * sizeof(float));
    
    int *a_i = (int *)aligned_alloc(16, total_size * sizeof(int));
    int *b_i = (int *)aligned_alloc(16, total_size * sizeof(int));
    int *c_i = (int *)aligned_alloc(16, total_size * sizeof(int));
    
    double *a_d = (double *)aligned_alloc(64, total_size * sizeof(double));
    double *b_d = (double *)aligned_alloc(64, total_size * sizeof(double));
    double *c_d = (double *)aligned_alloc(64, total_size * sizeof(double));
    
    if (!a_f || !b_f || !c_f || !a_i || !b_i || !c_i || !a_d || !b_d || !c_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterned data */
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < total_size; i++) {
        a_f[i] = (float)i;
        b_f[i] = (float)(total_size - i);
        c_f[i] = 0.0f;
        
        a_i[i] = i % 256;
        b_i[i] = (i * 3) % 256;
        c_i[i] = 0;
        
        a_d[i] = (double)i * 0.5;
        b_d[i] = (double)(total_size - i) * 0.25;
        c_d[i] = 0.0;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("\nTest 1: Target SIMD\n");
    test_target_simd(a_f, b_f, c_f, total_size);
    long long checksum1 = compute_checksum(c_f, total_size);
    printf("Checksum 1: %lld\n", checksum1);
    
    /* Test 2: Parallel for SIMD */
    printf("\nTest 2: Parallel for SIMD\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    long long checksum2 = compute_checksum_int(c_i, total_size);
    printf("Checksum 2: %lld\n", checksum2);
    
    /* Test 3: Nested SIMD */
    printf("\nTest 3: Nested SIMD\n");
    test_nested_simd(a_d, b_d, c_d, N);
    long long checksum3 = compute_checksum_double(c_d, total_size);
    printf("Checksum 3: %lld\n", checksum3);
    
    /* Test 4: Mixed directives */
    printf("\nTest 4: Mixed directives\n");
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < total_size; i++) {
        c_f[i] = 0.0f;  /* Reset */
    }
    test_mixed_simd(a_f, b_f, c_f, N);
    long long checksum4 = compute_checksum(c_f, total_size);
    printf("Checksum 4: %lld\n", checksum4);
    
    /* Test 5: Dynamic SIMD */
    printf("\nTest 5: Dynamic SIMD\n");
    test_dynamic_simd(total_size / 2);
    
    /* Validation: Compare host-only results if we ran both paths */
    if (use_gpu_offload) {
        printf("\nGPU offloading path executed\n");
        /* For full validation, you would need to run again without --use-gpu
           and compare checksums */
    } else {
        printf("\nHost-only SIMD path executed\n");
    }
    
    /* Cleanup */
    free(a_f); free(b_f); free(c_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    
    return 0;
}
