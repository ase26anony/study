#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64

// Global flag to control GPU offloading
static int use_gpu_offload = 0;

// Function declared with SIMD attribute
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_add(float a, float b, int i) {
    return a + b + (i * 0.001f);
}

// Test 1: Target SIMD with conditional offloading
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) num_teams(4) thread_limit(128)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    } else {
        // Host fallback
        #pragma omp simd simdlen(4) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

// Test 2: Parallel for SIMD with various clauses
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c: 64) \
        private(a, b, c) schedule(static, 32)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
    }
}

// Test 3: Nested SIMD with collapse
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4) \
            linear(i, j:1) reduction(+:c[0:n*m])
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] += simd_add(a[idx], b[idx], idx);
            }
        }
    }
}

// Test 4: Target SIMD with reduction
void test_target_simd_reduction(float *a, float *b, float *result, int n) {
    float sum = 0.0f;
    
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(tofrom: sum) \
            reduction(+:sum) simdlen(4) collapse(2)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < 2; j++) {
                sum += a[i] * b[i] + j;
            }
        }
    } else {
        #pragma omp parallel for simd reduction(+:sum) \
            simdlen(4) collapse(2)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < 2; j++) {
                sum += a[i] * b[i] + j;
            }
        }
    }
    
    *result = sum;
}

// Test 5: Dynamic arrays with pointer arithmetic
void test_dynamic_simd(int size) {
    float *dyn_a = (float*)malloc(size * sizeof(float));
    float *dyn_b = (float*)malloc(size * sizeof(float));
    float *dyn_c = (float*)malloc(size * sizeof(float));
    
    // Initialize with pattern
    #pragma omp simd simdlen(4)
    for (int i = 0; i < size; i++) {
        dyn_a[i] = i * 1.5f;
        dyn_b[i] = size - i;
    }
    
    if (use_gpu_offload) {
        #pragma omp target data map(to: dyn_a[0:size], dyn_b[0:size]) \
                               map(from: dyn_c[0:size])
        {
            #pragma omp target teams distribute parallel for simd \
                simdlen(8) safelen(32)
            for (int i = 0; i < size; i++) {
                dyn_c[i] = dyn_a[i] * dyn_b[i];
            }
        }
    } else {
        #pragma omp parallel for simd simdlen(8)
        for (int i = 0; i < size; i++) {
            dyn_c[i] = dyn_a[i] * dyn_b[i];
        }
    }
    
    // Compute checksum
    float checksum = 0.0f;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < size; i++) {
        checksum += dyn_c[i];
    }
    printf("Dynamic array checksum: %f\n", checksum);
    
    free(dyn_a);
    free(dyn_b);
    free(dyn_c);
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
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c = (float*)aligned_alloc(64, N * sizeof(float));
    
    int *a_int = (int*)aligned_alloc(64, N * sizeof(int));
    int *b_int = (int*)aligned_alloc(64, N * sizeof(int));
    int *c_int = (int*)aligned_alloc(64, N * sizeof(int));
    
    float *c_nested = (float*)calloc(N * M, sizeof(float));
    
    // Initialize with patterns
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0f;
        b[i] = N - i;
        a_int[i] = i;
        b_int[i] = N - i;
    }
    
    // Initialize nested arrays
    for (int i = 0; i < N * M; i++) {
        c_nested[i] = 0.0f;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    // Test 1: Target SIMD
    test_target_simd(a, b, c, N);
    
    // Compute checksum for validation
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N; i++) {
        checksum1 += c[i];
    }
    printf("Test 1 checksum: %f\n", checksum1);
    
    // Test 2: Parallel for SIMD
    test_parallel_for_simd(a_int, b_int, c_int, N);
    
    int checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N; i++) {
        checksum2 += c_int[i];
    }
    printf("Test 2 checksum: %d\n", checksum2);
    
    // Test 3: Nested SIMD
    test_nested_simd(a, b, c_nested, N, M);
    
    float checksum3 = 0.0f;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < N * M; i++) {
        checksum3 += c_nested[i];
    }
    printf("Test 3 checksum: %f\n", checksum3);
    
    // Test 4: Reduction
    float reduction_result;
    test_target_simd_reduction(a, b, &reduction_result, N);
    printf("Test 4 reduction result: %f\n", reduction_result);
    
    // Test 5: Dynamic arrays
    test_dynamic_simd(512);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(a_int);
    free(b_int);
    free(c_int);
    free(c_nested);
    
    printf("All tests completed.\n");
    
    return 0;
}
