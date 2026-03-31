#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64

int use_gpu_offload = 0;

// Function with declare simd pragma - will be vectorized
#pragma omp declare simd uniform(b) linear(i:1) notinbranch
float simd_func(float a, float b, int i) {
    return a * b + i * 0.5f;
}

// Test 1: Target SIMD with conditional offloading
void test_target_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        num_teams(4) thread_limit(128) \
        simdlen(4) safelen(8) \
        private(n) aligned(a, b, c: 32)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
}

// Test 2: Parallel for SIMD with various clauses
void test_parallel_for_simd(float *a, float *b, float *c, int n) {
    float sum = 0.0f;
    
    #pragma omp parallel for simd \
        reduction(+:sum) \
        simdlen(8) safelen(16) \
        schedule(static, 64) \
        aligned(a, b, c: 64) \
        linear(i:1) \
        private(i) shared(a, b, c)
    for (int i = 0; i < n; i++) {
        c[i] = simd_func(a[i], b[i], i);
        sum += c[i];
    }
    
    printf("Reduction sum: %.2f\n", sum);
}

// Test 3: Nested SIMD with collapse
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) \
            simdlen(4) \
            private(i, j) \
            ordered
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * b[idx] + (i + j) * 0.1f;
            }
        }
    }
}

// Test 4: Teams distribute parallel for SIMD with multiple clauses
void test_teams_distribute_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(tofrom: c[0:n]) \
        num_teams(8) \
        simdlen(4) safelen(16) \
        collapse(1) \
        dist_schedule(static, 128)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] / (b[i] + 1.0f);
    }
}

// Test 5: Mixed directives - SIMD inside parallel region
void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i += 64) {
            #pragma omp simd simdlen(4) aligned(a, b, c: 32)
            for (int j = i; j < i + 64 && j < n; j++) {
                c[j] = a[j] - b[j];
            }
        }
    }
}

// Validation function
int validate_results(float *c1, float *c2, int n, float tolerance) {
    for (int i = 0; i < n; i++) {
        if (fabs(c1[i] - c2[i]) > tolerance) {
            printf("Mismatch at index %d: %.6f vs %.6f\n", 
                   i, c1[i], c2[i]);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    // Parse command line argument for GPU offloading
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    // Allocate and initialize arrays
    float *a = (float *)aligned_alloc(64, N * M * sizeof(float));
    float *b = (float *)aligned_alloc(64, N * M * sizeof(float));
    float *c1 = (float *)aligned_alloc(64, N * M * sizeof(float));
    float *c2 = (float *)aligned_alloc(64, N * M * sizeof(float));
    float *c3 = (float *)aligned_alloc(64, N * M * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with patterned data
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < N * M; i++) {
        a[i] = (float)i;
        b[i] = (float)(N * M - i);
        c1[i] = 0.0f;
        c2[i] = 0.0f;
        c3[i] = 0.0f;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    // Test 1: Conditional target SIMD
    printf("\nTest 1: Target SIMD with conditional offloading\n");
    test_target_simd(a, b, c1, N * M);
    
    // Compute checksum to prevent dead code elimination
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1) simdlen(4)
    for (int i = 0; i < N * M; i++) {
        checksum1 += c1[i];
    }
    printf("Checksum 1: %.2f\n", checksum1);
    
    // Test 2: Parallel for SIMD (host-only)
    printf("\nTest 2: Parallel for SIMD\n");
    test_parallel_for_simd(a, b, c2, N * M);
    
    // Test 3: Nested SIMD
    printf("\nTest 3: Nested SIMD with collapse\n");
    test_nested_simd(a, b, c3, N, M);
    
    // Test 4: Teams distribute SIMD
    printf("\nTest 4: Teams distribute parallel for SIMD\n");
    test_teams_distribute_simd(a, b, c1, N * M);
    
    // Test 5: Mixed directives
    printf("\nTest 5: Mixed directives\n");
    test_mixed_directives(a, b, c2, N * M);
    
    // Run GPU version if requested
    if (use_gpu_offload) {
        printf("\nRunning GPU offloaded version...\n");
        int saved_flag = use_gpu_offload;
        
        // Run with GPU offloading
        use_gpu_offload = 1;
        float *gpu_result = (float *)aligned_alloc(64, N * M * sizeof(float));
        memset(gpu_result, 0, N * M * sizeof(float));
        test_target_simd(a, b, gpu_result, N * M);
        
        // Run without GPU offloading
        use_gpu_offload = 0;
        float *cpu_result = (float *)aligned_alloc(64, N * M * sizeof(float));
        memset(cpu_result, 0, N * M * sizeof(float));
        test_target_simd(a, b, cpu_result, N * M);
        
        // Validate results
        printf("\nValidating GPU vs CPU results...\n");
        if (validate_results(gpu_result, cpu_result, N * M, 1e-6f)) {
            printf("Validation PASSED\n");
        } else {
            printf("Validation FAILED\n");
        }
        
        free(gpu_result);
        free(cpu_result);
        use_gpu_offload = saved_flag;
    }
    
    // Final checksum to ensure all computations are used
    float final_checksum = 0.0f;
    #pragma omp parallel for simd reduction(+:final_checksum) \
        simdlen(8) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            final_checksum += c1[idx] + c2[idx] + c3[idx];
        }
    }
    printf("\nFinal checksum: %.2f\n", final_checksum);
    
    // Cleanup
    free(a);
    free(b);
    free(c1);
    free(c2);
    free(c3);
    
    return 0;
}
