#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

static int use_gpu_offload = 0;

// Function declared with SIMD attribute
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_add(float a, float b, int i) {
    return a + b + (i * 0.1f);
}

// Test 1: Target teams distribute parallel for simd with conditional offloading
void test_target_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        num_teams(4) thread_limit(128) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(4) safelen(8) aligned(a, b, c: 16) \
        private(n) reduction(+:n)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
        n += i; // dummy reduction
    }
}

// Test 2: Parallel for simd with various clauses
void test_parallel_for_simd(float *a, float *b, float *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) \
        aligned(a, b, c: 32) \
        linear(i:1) schedule(static, 64)
    for (int i = 0; i < n; i++) {
        c[i] = simd_add(a[i], b[i], i);
    }
}

// Test 3: Nested loops with collapse
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    float *tmp = (float*)malloc(n * m * sizeof(float));
    
    #pragma omp target teams distribute parallel for simd collapse(2) \
        if(target: use_gpu_offload) \
        map(to: a[0:n*m], b[0:n*m]) map(from: tmp[0:n*m]) \
        simdlen(4)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            tmp[idx] = a[idx] * b[idx];
        }
    }
    
    #pragma omp parallel for simd collapse(2) \
        simdlen(2) safelen(4)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            c[idx] = tmp[idx] + a[idx] + b[idx];
        }
    }
    
    free(tmp);
}

// Test 4: Mixed directives - simd inside parallel region
void test_mixed_simd(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for simd nowait \
            simdlen(4) aligned(a, b, c: 16)
        for (int i = 0; i < n/2; i++) {
            c[i] = a[i] - b[i];
        }
        
        #pragma omp for simd \
            simdlen(8) safelen(16)
        for (int i = n/2; i < n; i++) {
            c[i] = b[i] - a[i];
        }
    }
}

// Test 5: SIMD with reduction and lastprivate
void test_simd_reduction(float *a, float *b, float *c, int n) {
    float sum = 0.0f;
    int last_i = 0;
    
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        reduction(+:sum) lastprivate(last_i) \
        simdlen(4) safelen(8)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
        sum += c[i];
        last_i = i;
    }
    
    printf("Reduction sum: %f, last_i: %d\n", sum, last_i);
}

// Validation function
int validate_results(float *c1, float *c2, int n, const char *test_name) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (fabs(c1[i] - c2[i]) > 1e-6) {
            errors++;
            if (errors < 5) {
                printf("  Mismatch at %d: %f != %f\n", i, c1[i], c2[i]);
            }
        }
    }
    if (errors > 0) {
        printf("%s: %d errors found\n", test_name, errors);
    }
    return errors;
}

int main(int argc, char **argv) {
    // Parse command line argument
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
        }
    }
    
    printf("GPU offloading %s\n", use_gpu_offload ? "enabled" : "disabled");
    
    // Allocate and initialize arrays
    float *a = (float*)aligned_alloc(32, N * sizeof(float));
    float *b = (float*)aligned_alloc(32, N * sizeof(float));
    float *c_host = (float*)aligned_alloc(32, N * sizeof(float));
    float *c_gpu = (float*)aligned_alloc(32, N * sizeof(float));
    float *c_tmp = (float*)aligned_alloc(32, N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0f;
        b[i] = (N - i) * 1.0f;
        c_host[i] = 0.0f;
        c_gpu[i] = 0.0f;
        c_tmp[i] = 0.0f;
    }
    
    int total_errors = 0;
    
    // Test 1: Target SIMD with conditional execution
    printf("\n=== Test 1: Target SIMD ===\n");
    
    // Run with GPU offloading if enabled
    if (use_gpu_offload) {
        test_target_simd(a, b, c_gpu, N);
        
        // Also run host version for comparison
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        test_target_simd(a, b, c_host, N);
        use_gpu_offload = saved_flag;
        
        total_errors += validate_results(c_host, c_gpu, N, "Test 1");
    } else {
        test_target_simd(a, b, c_host, N);
    }
    
    // Compute checksum
    float checksum = 0.0f;
    #pragma omp simd reduction(+:checksum) simdlen(4)
    for (int i = 0; i < N; i++) {
        checksum += c_host[i];
    }
    printf("Checksum: %f\n", checksum);
    
    // Test 2: Parallel for SIMD (host-only)
    printf("\n=== Test 2: Parallel for SIMD ===\n");
    test_parallel_for_simd(a, b, c_tmp, N);
    
    checksum = 0.0f;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < N; i++) {
        checksum += c_tmp[i];
    }
    printf("Checksum: %f\n", checksum);
    
    // Test 3: Nested SIMD
    printf("\n=== Test 3: Nested SIMD ===\n");
    float *a2d = (float*)malloc(N * M * sizeof(float));
    float *b2d = (float*)malloc(N * M * sizeof(float));
    float *c2d = (float*)malloc(N * M * sizeof(float));
    
    for (int i = 0; i < N * M; i++) {
        a2d[i] = (i % 100) * 0.1f;
        b2d[i] = (i % 50) * 0.2f;
    }
    
    test_nested_simd(a2d, b2d, c2d, N, M);
    
    checksum = 0.0f;
    #pragma omp simd reduction(+:checksum) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += c2d[i * M + j];
        }
    }
    printf("Checksum: %f\n", checksum);
    
    // Test 4: Mixed directives
    printf("\n=== Test 4: Mixed directives ===\n");
    test_mixed_simd(a, b, c_tmp, N);
    
    checksum = 0.0f;
    #pragma omp parallel for simd reduction(+:checksum)
    for (int i = 0; i < N; i++) {
        checksum += c_tmp[i];
    }
    printf("Checksum: %f\n", checksum);
    
    // Test 5: SIMD with reduction
    printf("\n=== Test 5: SIMD with reduction ===\n");
    if (use_gpu_offload) {
        test_simd_reduction(a, b, c_gpu, N);
        
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        test_simd_reduction(a, b, c_host, N);
        use_gpu_offload = saved_flag;
        
        total_errors += validate_results(c_host, c_gpu, N, "Test 5");
    } else {
        test_simd_reduction(a, b, c_host, N);
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c_host);
    free(c_gpu);
    free(c_tmp);
    free(a2d);
    free(b2d);
    free(c2d);
    
    printf("\nTotal validation errors: %d\n", total_errors);
    
    return total_errors > 0 ? 1 : 0;
}
