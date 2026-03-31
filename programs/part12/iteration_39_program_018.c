#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

static int use_gpu_offload = 0;

/* Declare SIMD function */
#pragma omp declare simd uniform(a, b) linear(i:1) notinbranch
float simd_func(float a, float b, int i) {
    return a * b + i * 0.5f;
}

/* Test 1: Target teams distribute parallel for simd with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(128) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) aligned(a, b, c: 32) \
            private(n) reduction(+:n)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_func(a[i], b[i], i);
        }
    } else {
        /* Host fallback version */
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_func(a[i], b[i], i);
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c: 64) \
        schedule(static, 64) private(n) \
        linear(i:1) reduction(*:n)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

/* Test 3: Nested loops with collapse */
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4) \
            aligned(a, b, c: 16) linear(i,j:1)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * 2.0f - b[idx] / 3.0f;
            }
        }
    }
}

/* Test 4: Mixed directives - simd inside for */
void test_mixed_directives(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            #pragma omp simd simdlen(2) safelen(4)
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                c[idx] = a[idx] + b[idx] * (j + 1);
            }
        }
    }
}

/* Test 5: Dynamic arrays with pointer arithmetic */
void test_dynamic_arrays(int size) {
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    int *arr3 = (int *)malloc(size * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize arrays */
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
    }
    
    /* Conditional offloading with dynamic arrays */
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: arr1[0:size], arr2[0:size]) map(from: arr3[0:size]) \
            simdlen(8) safelen(16)
        for (int i = 0; i < size; i++) {
            arr3[i] = arr1[i] + arr2[i] * 2;
        }
    } else {
        #pragma omp simd simdlen(8)
        for (int i = 0; i < size; i++) {
            arr3[i] = arr1[i] + arr2[i] * 2;
        }
    }
    
    /* Compute checksum */
    long long checksum = 0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < size; i++) {
        checksum += arr3[i];
    }
    printf("Dynamic arrays checksum: %lld\n", checksum);
    
    free(arr1);
    free(arr2);
    free(arr3);
}

/* Test 6: Multiple SIMD regions with different data types */
void test_multi_type_simd() {
    short s_arr1[N], s_arr2[N], s_arr3[N];
    float f_arr1[N], f_arr2[N], f_arr3[N];
    
    /* Initialize */
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        s_arr1[i] = i % 256;
        s_arr2[i] = (N - i) % 256;
        f_arr1[i] = i * 0.5f;
        f_arr2[i] = (N - i) * 0.25f;
    }
    
    /* Process shorts */
    #pragma omp target simd map(to: s_arr1, s_arr2) map(from: s_arr3) \
        if(use_gpu_offload) simdlen(16)
    for (int i = 0; i < N; i++) {
        s_arr3[i] = s_arr1[i] + s_arr2[i];
    }
    
    /* Process floats */
    #pragma omp parallel for simd simdlen(8) aligned(f_arr1, f_arr2, f_arr3: 32)
    for (int i = 0; i < N; i++) {
        f_arr3[i] = f_arr1[i] * f_arr2[i] - i;
    }
    
    /* Verify */
    int short_sum = 0;
    float float_sum = 0.0f;
    #pragma omp simd reduction(+:short_sum, float_sum)
    for (int i = 0; i < N; i++) {
        short_sum += s_arr3[i];
        float_sum += f_arr3[i];
    }
    printf("Short sum: %d, Float sum: %.2f\n", short_sum, float_sum);
}

int main(int argc, char *argv[]) {
    /* Parse command line argument for GPU offloading */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    /* Allocate and initialize test data */
    float *a_f = (float *)aligned_alloc(32, N * sizeof(float));
    float *b_f = (float *)aligned_alloc(32, N * sizeof(float));
    float *c_f = (float *)aligned_alloc(32, N * sizeof(float));
    
    int *a_i = (int *)aligned_alloc(64, N * sizeof(int));
    int *b_i = (int *)aligned_alloc(64, N * sizeof(int));
    int *c_i = (int *)aligned_alloc(64, N * sizeof(int));
    
    double *a_d = (double *)malloc(N * 4 * sizeof(double));
    double *b_d = (double *)malloc(N * 4 * sizeof(double));
    double *c_d = (double *)malloc(N * 4 * sizeof(double));
    
    if (!a_f || !b_f || !c_f || !a_i || !b_i || !c_i || !a_d || !b_d || !c_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data with patterns */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 1.5f;
        b_f[i] = (N - i) * 0.75f;
        a_i[i] = i;
        b_i[i] = N - i;
    }
    
    for (int i = 0; i < N * 4; i++) {
        a_d[i] = i * 0.25;
        b_d[i] = (N * 4 - i) * 0.5;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    /* Run test suite */
    test_target_simd(a_f, b_f, c_f, N);
    
    /* Compute checksum for verification */
    float checksum_f = 0.0f;
    #pragma omp simd reduction(+:checksum_f)
    for (int i = 0; i < N; i++) {
        checksum_f += c_f[i];
    }
    printf("Test 1 checksum: %.2f\n", checksum_f);
    
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    int checksum_i = 0;
    #pragma omp simd reduction(+:checksum_i)
    for (int i = 0; i < N; i++) {
        checksum_i += c_i[i];
    }
    printf("Test 2 checksum: %d\n", checksum_i);
    
    test_nested_simd(a_f, b_f, c_f, 32, 32);
    
    test_mixed_directives(a_d, b_d, c_d, N);
    
    test_dynamic_arrays(512);
    
    test_multi_type_simd();
    
    /* Cleanup */
    free(a_f); free(b_f); free(c_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    
    printf("All tests completed.\n");
    return 0;
}
