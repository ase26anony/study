#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64

// Global flag to control GPU offloading
static int use_gpu_offload = 0;

// Function with declare simd directive
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_func(float a, float b, int i) {
    return a * b + i * 0.5f;
}

// Test 1: Target SIMD with conditional offloading
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        #pragma omp simd simdlen(4) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

// Test 2: Parallel for SIMD with various clauses
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(8) safelen(16) \
        aligned(a, b, c: 64) linear(i:1) private(i)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

// Test 3: Nested SIMD with collapse
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = simd_func(a[idx], b[idx], idx);
            }
        }
    }
}

// Test 4: Teams distribute with SIMD
void test_teams_distribute_simd(double *a, double *b, double *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        num_teams(4) thread_limit(128) simdlen(2)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] / (b[i] + 1.0);
    }
}

// Test 5: Mixed directives with dynamic arrays
void test_mixed_directives(int size) {
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    int *result = (int *)malloc(size * sizeof(int));
    
    // Initialize
    #pragma omp parallel for simd
    for (int i = 0; i < size; i++) {
        arr1[i] = i % 100;
        arr2[i] = (size - i) % 100;
    }
    
    // Compute with different SIMD constructs
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: arr1[0:size], arr2[0:size]) map(from: result[0:size]) \
            simdlen(4) collapse(1)
        for (int i = 0; i < size; i++) {
            result[i] = arr1[i] * 2 + arr2[i];
        }
    } else {
        #pragma omp for simd simdlen(8)
        for (int i = 0; i < size; i++) {
            result[i] = arr1[i] * 2 + arr2[i];
        }
    }
    
    // Checksum
    int checksum = 0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < size; i++) {
        checksum += result[i];
    }
    printf("Mixed directives checksum: %d\n", checksum);
    
    free(arr1);
    free(arr2);
    free(result);
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
    float *a_f = (float *)malloc(N * sizeof(float));
    float *b_f = (float *)malloc(N * sizeof(float));
    float *c_f = (float *)malloc(N * sizeof(float));
    
    int *a_i = (int *)malloc(N * sizeof(int));
    int *b_i = (int *)malloc(N * sizeof(int));
    int *c_i = (int *)malloc(N * sizeof(int));
    
    double *a_d = (double *)malloc(N * sizeof(double));
    double *b_d = (double *)malloc(N * sizeof(double));
    double *c_d = (double *)malloc(N * sizeof(double));
    
    // Initialize with patterned data
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 1.0f;
        b_f[i] = (N - i) * 1.0f;
        a_i[i] = i;
        b_i[i] = N - i;
        a_d[i] = i * 0.5;
        b_d[i] = (N - i) * 0.5;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    // Test 1: Target SIMD with conditional execution
    printf("\nTest 1: Target SIMD\n");
    test_target_simd(a_f, b_f, c_f, N);
    
    // Compute checksum
    float sum_f = 0.0f;
    #pragma omp simd reduction(+:sum_f)
    for (int i = 0; i < N; i++) {
        sum_f += c_f[i];
    }
    printf("Target SIMD checksum: %f\n", sum_f);
    
    // Test 2: Parallel for SIMD
    printf("\nTest 2: Parallel for SIMD\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    int sum_i = 0;
    #pragma omp simd reduction(+:sum_i)
    for (int i = 0; i < N; i++) {
        sum_i += c_i[i];
    }
    printf("Parallel for SIMD checksum: %d\n", sum_i);
    
    // Test 3: Nested SIMD
    printf("\nTest 3: Nested SIMD\n");
    test_nested_simd(a_f, b_f, c_f, N/2, 2);
    
    sum_f = 0.0f;
    #pragma omp simd reduction(+:sum_f)
    for (int i = 0; i < N; i++) {
        sum_f += c_f[i];
    }
    printf("Nested SIMD checksum: %f\n", sum_f);
    
    // Test 4: Teams distribute SIMD (always offloaded if GPU enabled)
    printf("\nTest 4: Teams distribute SIMD\n");
    if (use_gpu_offload) {
        test_teams_distribute_simd(a_d, b_d, c_d, N);
        
        double sum_d = 0.0;
        #pragma omp simd reduction(+:sum_d)
        for (int i = 0; i < N; i++) {
            sum_d += c_d[i];
        }
        printf("Teams distribute SIMD checksum: %f\n", sum_d);
    }
    
    // Test 5: Mixed directives with dynamic allocation
    printf("\nTest 5: Mixed directives\n");
    test_mixed_directives(512);
    
    // Validation: Compare GPU vs CPU results if both paths were tested
    if (use_gpu_offload) {
        printf("\nValidation: GPU offloading path was used\n");
        // In a real scenario, you would run both paths and compare
        printf("(To compare with CPU path, run without --use-gpu flag)\n");
    } else {
        printf("\nValidation: CPU-only path was used\n");
    }
    
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
    
    printf("\nAll tests completed.\n");
    return 0;
}
