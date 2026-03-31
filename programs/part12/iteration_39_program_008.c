#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

static int use_gpu_offload = 0;

// Function with declare simd pragma
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_add(float a, float b, int i) {
    return a + b + (i * 0.1f);
}

// Test 1: Target teams distribute parallel for simd with conditional execution
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(128) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) aligned(a, b, c: 32) \
            private(n) reduction(+:n)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_add(a[i], b[i], i);
        }
    } else {
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

// Test 2: Parallel for simd with various clauses
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c: 64) \
        linear(i:1) private(i) schedule(static, 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

// Test 3: Nested loops with collapse
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4) \
            aligned(a, b, c: 32) private(i, j)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * 2.0f - b[idx] / 3.0f;
            }
        }
    }
}

// Test 4: Mixed directives - simd inside for
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

// Test 5: Target simd with dynamic arrays and pointer arithmetic
void test_dynamic_arrays(int size) {
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    int *arr3 = (int *)malloc(size * sizeof(int));
    
    // Initialize arrays
    #pragma omp simd simdlen(4)
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
    }
    
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: arr1[0:size], arr2[0:size]) map(from: arr3[0:size]) \
            simdlen(8) device(0) num_teams(8)
        for (int i = 0; i < size; i++) {
            arr3[i] = arr1[i] + arr2[i] * 2;
        }
    } else {
        #pragma omp parallel for simd simdlen(8)
        for (int i = 0; i < size; i++) {
            arr3[i] = arr1[i] + arr2[i] * 2;
        }
    }
    
    // Checksum
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < size; i++) {
        sum += arr3[i];
    }
    printf("Dynamic arrays checksum: %d\n", sum);
    
    free(arr1);
    free(arr2);
    free(arr3);
}

// Test 6: Complex reduction with simd
float test_reduction_simd(float *data, int n) {
    float sum = 0.0f;
    
    #pragma omp parallel for simd reduction(+:sum) \
        simdlen(4) safelen(16) aligned(data: 32)
    for (int i = 0; i < n; i++) {
        sum += data[i] * data[i];
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    // Parse command line argument for GPU offloading
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    // Allocate and initialize arrays
    float *a_f = (float *)aligned_alloc(32, N * sizeof(float));
    float *b_f = (float *)aligned_alloc(32, N * sizeof(float));
    float *c_f = (float *)aligned_alloc(32, N * sizeof(float));
    
    int *a_i = (int *)aligned_alloc(64, N * sizeof(int));
    int *b_i = (int *)aligned_alloc(64, N * sizeof(int));
    int *c_i = (int *)aligned_alloc(64, N * sizeof(int));
    
    double *a_d = (double *)aligned_alloc(32, N * 4 * sizeof(double));
    double *b_d = (double *)aligned_alloc(32, N * 4 * sizeof(double));
    double *c_d = (double *)aligned_alloc(32, N * 4 * sizeof(double));
    
    // Initialize with patterned data
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 1.5f;
        b_f[i] = N - i * 0.5f;
        a_i[i] = i;
        b_i[i] = N - i;
    }
    
    for (int i = 0; i < N * 4; i++) {
        a_d[i] = i * 0.25;
        b_d[i] = (N * 4 - i) * 0.5;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    // Run test suite
    test_target_simd(a_f, b_f, c_f, N);
    
    // Compute checksum for validation
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N; i++) {
        checksum1 += c_f[i];
    }
    printf("Test 1 checksum: %f\n", checksum1);
    
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    int checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N; i++) {
        checksum2 += c_i[i];
    }
    printf("Test 2 checksum: %d\n", checksum2);
    
    test_nested_simd(a_f, b_f, c_f, N/2, 2);
    
    float checksum3 = 0.0f;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < N; i++) {
        checksum3 += c_f[i];
    }
    printf("Test 3 checksum: %f\n", checksum3);
    
    test_mixed_directives(a_d, b_d, c_d, N);
    
    double checksum4 = 0.0;
    #pragma omp simd reduction(+:checksum4)
    for (int i = 0; i < N * 4; i++) {
        checksum4 += c_d[i];
    }
    printf("Test 4 checksum: %lf\n", checksum4);
    
    test_dynamic_arrays(512);
    
    float reduction_result = test_reduction_simd(a_f, N);
    printf("Reduction test result: %f\n", reduction_result);
    
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
    
    return 0;
}
