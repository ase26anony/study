#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

static int use_gpu_offload = 0;

#pragma omp declare simd uniform(a, b) linear(i:1) notinbranch
float simd_function(float a, float b, int i) {
    return a * b + i * 0.5f;
}

void test_target_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(4) safelen(8) aligned(a, b, c: 32) \
        private(i) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
    
    // Additional SIMD region with different clauses
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(tofrom: c[0:n]) \
        simdlen(8) collapse(2) \
        linear(i, j:1)
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            c[idx] = simd_function(c[idx], 1.5f, idx);
        }
    }
}

void test_parallel_for_simd(float *a, float *b, float *c, int n) {
    float sum = 0.0f;
    
    #pragma omp parallel for simd \
        simdlen(4) safelen(16) aligned(a, b, c: 16) \
        reduction(+:sum) schedule(static, 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
        sum += c[i];
    }
    
    printf("Host SIMD sum: %f\n", sum);
    
    // Nested SIMD with collapse
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(2) \
            private(i, j) aligned(a, b: 32)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                if (idx < n) {
                    c[idx] = a[idx] - b[idx];
                }
            }
        }
    }
}

void test_nested_simd(float *a, float *b, float *c, int n) {
    // Complex nested structure
    #pragma omp target teams if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    {
        #pragma omp distribute simd dist_schedule(static, 16) \
            simdlen(4)
        for (int i = 0; i < n; i += 16) {
            #pragma omp parallel for simd simdlen(2) \
                private(j) reduction(max:max_val)
            for (int j = i; j < i + 16 && j < n; j++) {
                c[j] = a[j] / (b[j] + 1.0f);
                if (c[j] > max_val) max_val = c[j];
            }
        }
    }
}

void test_mixed_directives(float *a, float *b, float *c, int n) {
    // Mix of for and simd directives
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i += 64) {
            #pragma omp simd simdlen(8) aligned(a, b, c: 64) \
                linear(k:1)
            for (int k = i; k < i + 64 && k < n; k++) {
                c[k] = a[k] + b[k] * 0.5f;
            }
        }
        
        #pragma omp for simd simdlen(4) collapse(2) \
            private(i, j) schedule(dynamic, 8)
        for (int i = 0; i < n/4; i++) {
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < n) {
                    c[idx] += simd_function(a[idx], b[idx], idx);
                }
            }
        }
    }
}

float compute_checksum(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    // Parse command line argument
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
        }
    }
    
    // Allocate aligned memory
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c = (float*)aligned_alloc(64, N * sizeof(float));
    float *c_host = (float*)aligned_alloc(64, N * sizeof(float));
    
    // Initialize arrays
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
        c[i] = 0.0f;
        c_host[i] = 0.0f;
    }
    
    printf("Testing with GPU offload: %s\n", use_gpu_offload ? "YES" : "NO");
    
    // Test 1: Target SIMD with conditional offloading
    printf("\n=== Test 1: Target SIMD ===\n");
    memcpy(c, c_host, N * sizeof(float));
    test_target_simd(a, b, c, N);
    float checksum1 = compute_checksum(c, N);
    printf("Checksum 1: %f\n", checksum1);
    
    // Test 2: Parallel for SIMD (host-only)
    printf("\n=== Test 2: Parallel for SIMD ===\n");
    memcpy(c, c_host, N * sizeof(float));
    test_parallel_for_simd(a, b, c, N);
    float checksum2 = compute_checksum(c, N);
    printf("Checksum 2: %f\n", checksum2);
    
    // Test 3: Nested SIMD
    printf("\n=== Test 3: Nested SIMD ===\n");
    memcpy(c, c_host, N * sizeof(float));
    test_nested_simd(a, b, c, N);
    float checksum3 = compute_checksum(c, N);
    printf("Checksum 3: %f\n", checksum3);
    
    // Test 4: Mixed directives
    printf("\n=== Test 4: Mixed Directives ===\n");
    memcpy(c, c_host, N * sizeof(float));
    test_mixed_directives(a, b, c, N);
    float checksum4 = compute_checksum(c, N);
    printf("Checksum 4: %f\n", checksum4);
    
    // Run again with opposite offload setting for comparison
    if (use_gpu_offload) {
        printf("\n=== Running host-only version for comparison ===\n");
        int saved_setting = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c_ref = (float*)aligned_alloc(64, N * sizeof(float));
        memcpy(c_ref, c_host, N * sizeof(float));
        test_target_simd(a, b, c_ref, N);
        
        // Compare results
        int errors = 0;
        #pragma omp parallel for simd reduction(+:errors) simdlen(4)
        for (int i = 0; i < N; i++) {
            if (fabsf(c[i] - c_ref[i]) > 1e-5f) {
                errors++;
            }
        }
        
        if (errors == 0) {
            printf("GPU and CPU results match!\n");
        } else {
            printf("Mismatch found in %d elements\n", errors);
        }
        
        free(c_ref);
        use_gpu_offload = saved_setting;
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(c_host);
    
    return 0;
}
