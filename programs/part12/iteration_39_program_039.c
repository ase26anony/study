#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64

// Global flag to control GPU offloading
static int use_gpu_offload = 0;

// Function with declare simd pragma
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_add(float a, float b, int i) {
    return a + b + (i * 0.001f);
}

// Test 1: Target teams distribute parallel for simd with conditional execution
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(128) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
            sum += c[i];
        }
        printf("GPU offload sum: %f\n", sum);
    } else {
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c:32) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

// Test 2: Parallel for simd with various clauses
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c:64) \
        schedule(static, 64) private(i) collapse(1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

// Test 3: Nested loops with collapse clause
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4) private(i, j)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = simd_add(a[idx], b[idx], idx);
            }
        }
    }
}

// Test 4: Mixed directives - simd inside for
void test_mixed_simd(double *a, double *b, double *c, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        #pragma omp simd simdlen(2) aligned(a, b, c:16)
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            c[idx] = a[idx] * 2.0 + b[idx];
        }
    }
}

// Test 5: Dynamic arrays with pointer-based access
void test_dynamic_simd(int size) {
    int *x = (int *)malloc(size * sizeof(int));
    int *y = (int *)malloc(size * sizeof(int));
    int *z = (int *)malloc(size * sizeof(int));
    
    // Initialize
    #pragma omp simd simdlen(4)
    for (int i = 0; i < size; i++) {
        x[i] = i;
        y[i] = size - i;
    }
    
    // Compute with target simd if GPU offload enabled
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: x[0:size], y[0:size]) map(from: z[0:size]) \
            simdlen(8) num_teams(2)
        for (int i = 0; i < size; i++) {
            z[i] = x[i] * y[i] + i;
        }
    } else {
        #pragma omp parallel for simd simdlen(8) aligned(x, y, z:32)
        for (int i = 0; i < size; i++) {
            z[i] = x[i] * y[i] + i;
        }
    }
    
    // Checksum
    int checksum = 0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < size; i++) {
        checksum += z[i];
    }
    printf("Dynamic array checksum: %d\n", checksum);
    
    free(x);
    free(y);
    free(z);
}

int main(int argc, char *argv[]) {
    // Parse command line argument for GPU offloading
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    // Set environment variable for runtime check (mimics IFN_GOMP_USE_SIMT)
    if (use_gpu_offload) {
        setenv("OMP_TARGET_OFFLOAD", "MANDATORY", 1);
    }
    
    // Allocate and initialize arrays
    float *a_f = (float *)aligned_alloc(32, N * sizeof(float));
    float *b_f = (float *)aligned_alloc(32, N * sizeof(float));
    float *c_f = (float *)aligned_alloc(32, N * sizeof(float));
    
    int *a_i = (int *)aligned_alloc(64, N * sizeof(int));
    int *b_i = (int *)aligned_alloc(64, N * sizeof(int));
    int *c_i = (int *)aligned_alloc(64, N * sizeof(int));
    
    double *a_d = (double *)aligned_alloc(16, N * 4 * sizeof(double));
    double *b_d = (double *)aligned_alloc(16, N * 4 * sizeof(double));
    double *c_d = (double *)aligned_alloc(16, N * 4 * sizeof(double));
    
    // Initialize with patterned data
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 0.5f;
        b_f[i] = (N - i) * 0.25f;
        a_i[i] = i;
        b_i[i] = N - i;
    }
    
    for (int i = 0; i < N * 4; i++) {
        a_d[i] = i * 0.1;
        b_d[i] = i * 0.2;
    }
    
    // Run test functions
    printf("Test 1: Target SIMD\n");
    test_target_simd(a_f, b_f, c_f, N);
    
    printf("Test 2: Parallel for SIMD\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    printf("Test 3: Nested SIMD\n");
    test_nested_simd(a_f, b_f, c_f, N/2, 2);
    
    printf("Test 4: Mixed directives\n");
    test_mixed_simd(a_d, b_d, c_d, N);
    
    printf("Test 5: Dynamic arrays\n");
    test_dynamic_simd(512);
    
    // Validation: Compare results if both paths were taken
    if (use_gpu_offload) {
        // Re-run test 1 without GPU for comparison
        int temp_use_gpu = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c_f_host = (float *)aligned_alloc(32, N * sizeof(float));
        test_target_simd(a_f, b_f, c_f_host, N);
        
        // Compare results
        int errors = 0;
        #pragma omp simd reduction(+:errors)
        for (int i = 0; i < N; i++) {
            if (fabs(c_f[i] - c_f_host[i]) > 0.001f) {
                errors++;
            }
        }
        
        if (errors == 0) {
            printf("Validation PASSED: GPU and CPU results match\n");
        } else {
            printf("Validation FAILED: %d mismatches found\n", errors);
        }
        
        free(c_f_host);
        use_gpu_offload = temp_use_gpu;
    }
    
    // Compute final checksums to prevent dead code elimination
    float sum_f = 0.0f;
    int sum_i = 0;
    double sum_d = 0.0;
    
    #pragma omp simd reduction(+:sum_f)
    for (int i = 0; i < N; i++) {
        sum_f += c_f[i];
    }
    
    #pragma omp simd reduction(+:sum_i)
    for (int i = 0; i < N; i++) {
        sum_i += c_i[i];
    }
    
    #pragma omp simd reduction(+:sum_d)
    for (int i = 0; i < N * 4; i++) {
        sum_d += c_d[i];
    }
    
    printf("Final checksums - Float: %f, Int: %d, Double: %lf\n", 
           sum_f, sum_i, sum_d);
    
    // Cleanup
    free(a_f); free(b_f); free(c_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    
    return 0;
}
