#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

// Global flag to control GPU offloading
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
            private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_add(a[i], b[i], i);
        }
    } else {
        // Host fallback version
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32) \
                linear(i:1) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_add(a[i], b[i], i);
        }
    }
}

// Test 2: Parallel for simd with various clauses
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(8) safelen(16) \
            collapse(2) schedule(static) \
            aligned(a, b, c: 64) private(i, j) \
            lastprivate(last_i, last_j)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c[idx] = a[idx] * 2 + b[idx] * 3;
        }
    }
}

// Test 3: Nested SIMD with collapse clause
void test_nested_simd(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(2) \
                aligned(a, b, c: 16) linear(k:1)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int k = i * n + j;
                c[k] = a[k] * b[k] / (k + 1.0);
            }
        }
    }
}

// Test 4: Mixed directives - simd inside for
void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int block = 0; block < n; block += 64) {
            int end = (block + 64) < n ? (block + 64) : n;
            #pragma omp simd simdlen(4) safelen(8)
            for (int i = block; i < end; i++) {
                c[i] = (a[i] - b[i]) * (a[i] + b[i]);
            }
        }
    }
}

// Test 5: Dynamic arrays with pointer-based access
void test_dynamic_arrays(int size) {
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    int *arr3 = (int *)malloc(size * sizeof(int));
    
    // Initialize
    #pragma omp simd simdlen(4)
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
    }
    
    // Conditional target region
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: arr1[0:size], arr2[0:size]) \
            map(from: arr3[0:size]) \
            simdlen(8) collapse(1) \
            private(i) reduction(max:max_val)
        for (int i = 0; i < size; i++) {
            arr3[i] = arr1[i] + arr2[i] * 2;
        }
    } else {
        #pragma omp parallel for simd simdlen(8)
        for (int i = 0; i < size; i++) {
            arr3[i] = arr1[i] + arr2[i] * 2;
        }
    }
    
    free(arr1);
    free(arr2);
    free(arr3);
}

int main(int argc, char *argv[]) {
    // Parse command line argument for GPU offloading
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    // Set environment variable for runtime check
    if (use_gpu_offload) {
        setenv("OMP_TARGET_OFFLOAD", "MANDATORY", 1);
    }
    
    // Allocate and initialize arrays
    float *a_f = (float *)aligned_alloc(32, N * sizeof(float));
    float *b_f = (float *)aligned_alloc(32, N * sizeof(float));
    float *c_f = (float *)aligned_alloc(32, N * sizeof(float));
    
    int *a_i = (int *)aligned_alloc(64, N * M * sizeof(int));
    int *b_i = (int *)aligned_alloc(64, N * M * sizeof(int));
    int *c_i = (int *)aligned_alloc(64, N * M * sizeof(int));
    
    double *a_d = (double *)aligned_alloc(16, N * N * sizeof(double));
    double *b_d = (double *)aligned_alloc(16, N * N * sizeof(double));
    double *c_d = (double *)aligned_alloc(16, N * N * sizeof(double));
    
    // Initialize with patterned data
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 1.0f;
        b_f[i] = (N - i) * 1.0f;
    }
    
    #pragma omp simd
    for (int i = 0; i < N * M; i++) {
        a_i[i] = i % 100;
        b_i[i] = (i * 2) % 100;
    }
    
    #pragma omp simd simdlen(2)
    for (int i = 0; i < N * N; i++) {
        a_d[i] = i * 0.5;
        b_d[i] = i * 0.25;
    }
    
    // Run test functions
    printf("Running test_target_simd...\n");
    test_target_simd(a_f, b_f, c_f, N);
    
    // Compute checksum to prevent dead code elimination
    float checksum_f = 0.0f;
    #pragma omp simd reduction(+:checksum_f)
    for (int i = 0; i < N; i++) {
        checksum_f += c_f[i];
    }
    printf("Checksum test_target_simd: %f\n", checksum_f);
    
    printf("Running test_parallel_for_simd...\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    int checksum_i = 0;
    #pragma omp simd reduction(+:checksum_i)
    for (int i = 0; i < N * M; i++) {
        checksum_i += c_i[i];
    }
    printf("Checksum test_parallel_for_simd: %d\n", checksum_i);
    
    printf("Running test_nested_simd...\n");
    test_nested_simd(a_d, b_d, c_d, N);
    
    double checksum_d = 0.0;
    #pragma omp simd reduction(+:checksum_d)
    for (int i = 0; i < N * N; i++) {
        checksum_d += c_d[i];
    }
    printf("Checksum test_nested_simd: %lf\n", checksum_d);
    
    printf("Running test_mixed_directives...\n");
    test_mixed_directives(a_f, b_f, c_f, N);
    
    printf("Running test_dynamic_arrays...\n");
    test_dynamic_arrays(512);
    
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
