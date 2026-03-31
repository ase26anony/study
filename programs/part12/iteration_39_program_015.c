#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

// Global flag to control GPU offloading
static int use_gpu_offload = 0;

// Function declared with SIMD attribute
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_multiply(float a, float b, int i) {
    return a * b * (i % 10);
}

// Test 1: Target SIMD with conditional offloading
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(64) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(8) safelen(16) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        // Host fallback
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 16)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

// Test 2: Parallel for SIMD with various clauses
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(4) safelen(8) collapse(2) \
        private(a, b, c) schedule(static, 16)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c[idx] = a[idx] * b[idx] + j;
        }
    }
}

// Test 3: Nested SIMD with reduction
float test_nested_simd_reduction(float *a, float *b, int n) {
    float sum = 0.0f;
    
    #pragma omp parallel reduction(+:sum)
    {
        #pragma omp for simd collapse(2) \
            simdlen(4) linear(i,j:1) nowait
        for (int i = 0; i < n/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                sum += simd_multiply(a[idx], b[idx], idx);
            }
        }
    }
    return sum;
}

// Test 4: Teams distribute with SIMD and dynamic mapping
void test_teams_distribute_simd(double *a, double *b, double *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: c[0:n]) map(to: a[0:n], b[0:n]) \
        num_teams(8) thread_limit(128) \
        simdlen(4) safelen(32) collapse(1)
    for (int i = 0; i < n; i++) {
        c[i] = (a[i] * b[i]) / (i + 1.0);
    }
}

// Test 5: Mixed directives - SIMD inside parallel region
void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i += 64) {
            #pragma omp simd simdlen(8) aligned(a, b, c: 64)
            for (int j = i; j < i + 64 && j < n; j++) {
                c[j] = a[j] - b[j];
            }
        }
    }
}

// Test 6: SIMD with if clause and device-specific code
void test_conditional_simd(int *a, int *b, int *c, int n) {
    #pragma omp target simd if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < n; i++) {
        #ifdef __NVPTX__
        // GPU-specific optimization
        c[i] = __shfl_sync(0xffffffff, a[i] + b[i], i % 32);
        #else
        c[i] = a[i] + b[i];
        #endif
    }
}

// Validation function
int validate_results(float *c1, float *c2, int n, float tolerance) {
    for (int i = 0; i < n; i++) {
        if (fabs(c1[i] - c2[i]) > tolerance) {
            printf("Mismatch at index %d: %f != %f\n", i, c1[i], c2[i]);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    // Parse command line argument
    if (argc > 1 && strcmp(argv[1], "--use-gpu") == 0) {
        use_gpu_offload = 1;
        printf("GPU offloading enabled\n");
    } else {
        printf("GPU offloading disabled (host-only execution)\n");
    }
    
    // Allocate and initialize arrays
    float *a_f = (float*)malloc(N * sizeof(float));
    float *b_f = (float*)malloc(N * sizeof(float));
    float *c1_f = (float*)malloc(N * sizeof(float));
    float *c2_f = (float*)malloc(N * sizeof(float));
    
    int *a_i = (int*)malloc(N * M * sizeof(int));
    int *b_i = (int*)malloc(N * M * sizeof(int));
    int *c_i = (int*)malloc(N * M * sizeof(int));
    
    double *a_d = (double*)malloc(N * sizeof(double));
    double *b_d = (double*)malloc(N * sizeof(double));
    double *c_d = (double*)malloc(N * sizeof(double));
    
    // Initialize with patterned data
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 1.0f;
        b_f[i] = (N - i) * 1.0f;
        a_d[i] = i * 1.0;
        b_d[i] = (N - i) * 1.0;
    }
    
    for (int i = 0; i < N * M; i++) {
        a_i[i] = i % 100;
        b_i[i] = (i * 2) % 100;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    // Test 1: Target SIMD
    printf("\nTest 1: Target SIMD with conditional offloading\n");
    test_target_simd(a_f, b_f, c1_f, N);
    
    // Compute checksum
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N; i++) {
        checksum1 += c1_f[i];
    }
    printf("Checksum 1: %f\n", checksum1);
    
    // Test 2: Parallel for SIMD
    printf("\nTest 2: Parallel for SIMD with collapse\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    int checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N * M; i++) {
        checksum2 += c_i[i];
    }
    printf("Checksum 2: %d\n", checksum2);
    
    // Test 3: Nested SIMD with reduction
    printf("\nTest 3: Nested SIMD reduction\n");
    float reduction_result = test_nested_simd_reduction(a_f, b_f, N);
    printf("Reduction result: %f\n", reduction_result);
    
    // Test 4: Teams distribute SIMD (always offload if enabled)
    printf("\nTest 4: Teams distribute SIMD\n");
    if (use_gpu_offload) {
        test_teams_distribute_simd(a_d, b_d, c_d, N);
        
        double checksum4 = 0.0;
        #pragma omp simd reduction(+:checksum4)
        for (int i = 0; i < N; i++) {
            checksum4 += c_d[i];
        }
        printf("Checksum 4: %lf\n", checksum4);
    }
    
    // Test 5: Mixed directives
    printf("\nTest 5: Mixed directives (SIMD inside parallel)\n");
    test_mixed_directives(a_f, b_f, c2_f, N);
    
    float checksum5 = 0.0f;
    #pragma omp simd reduction(+:checksum5)
    for (int i = 0; i < N; i++) {
        checksum5 += c2_f[i];
    }
    printf("Checksum 5: %f\n", checksum5);
    
    // Test 6: Conditional SIMD
    printf("\nTest 6: Conditional SIMD with device-specific code\n");
    test_conditional_simd(a_i, b_i, c_i, N);
    
    int checksum6 = 0;
    #pragma omp simd reduction(+:checksum6)
    for (int i = 0; i < N; i++) {
        checksum6 += c_i[i];
    }
    printf("Checksum 6: %d\n", checksum6);
    
    // Validation between GPU and CPU paths
    if (use_gpu_offload) {
        // Re-run test 1 without GPU to compare
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c_cpu = (float*)malloc(N * sizeof(float));
        test_target_simd(a_f, b_f, c_cpu, N);
        
        use_gpu_offload = saved_flag;
        
        if (validate_results(c1_f, c_cpu, N, 1e-6f)) {
            printf("\nValidation PASSED: GPU and CPU results match\n");
        } else {
            printf("\nValidation FAILED: GPU and CPU results differ\n");
        }
        
        free(c_cpu);
    }
    
    // Cleanup
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
    
    printf("\nAll tests completed\n");
    return 0;
}
