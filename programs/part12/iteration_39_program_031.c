#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64

int use_gpu_offload = 0;

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
}

void test_parallel_for_simd(float *a, float *b, float *c, int n) {
    float sum = 0.0f;
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c: 64) \
        reduction(+:sum) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = simd_function(a[i], b[i], i);
        sum += c[i];
    }
    printf("Host SIMD sum: %f\n", sum);
}

void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for collapse(2) simd \
            simdlen(2) safelen(4) \
            private(i, j) lastprivate(last_i, last_j)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * 0.5f + b[idx] * 1.5f;
                last_i = i;
                last_j = j;
            }
        }
    }
}

void test_mixed_directives(float *a, float *b, float *c, int n) {
    // Mixed #pragma omp for and #pragma omp simd
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            #pragma omp simd simdlen(4)
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < n) {
                    c[idx] = a[idx] - b[idx];
                }
            }
        }
    }
}

void test_target_teams_simd(float *a, float *b, float *c, int n) {
    // Complex nesting: teams → distribute → parallel for → simd
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        num_teams(8) thread_limit(128) \
        simdlen(4) collapse(2) \
        private(i, j)
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            c[idx] = a[idx] * b[idx] + (float)(i + j);
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
    // Parse command line argument to determine execution path
    if (argc > 1 && strcmp(argv[1], "--use-gpu") == 0) {
        use_gpu_offload = 1;
        printf("GPU offloading enabled\n");
    } else {
        printf("Host-only execution\n");
    }
    
    // Allocate and initialize arrays with dynamic allocation
    float *a = (float *)aligned_alloc(64, N * sizeof(float));
    float *b = (float *)aligned_alloc(64, N * sizeof(float));
    float *c1 = (float *)aligned_alloc(64, N * sizeof(float));
    float *c2 = (float *)aligned_alloc(64, N * sizeof(float));
    float *c3 = (float *)aligned_alloc(64, N * sizeof(float));
    float *c4 = (float *)aligned_alloc(64, N * M * sizeof(float));
    
    // Patterned initialization
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
    }
    for (int i = 0; i < N * M; i++) {
        c4[i] = 0.0f;
    }
    
    printf("Array size: %d elements\n", N);
    
    // Test 1: Target SIMD with conditional offloading
    printf("\nTest 1: Target SIMD\n");
    memset(c1, 0, N * sizeof(float));
    test_target_simd(a, b, c1, N);
    float checksum1 = compute_checksum(c1, N);
    printf("Checksum 1: %f\n", checksum1);
    
    // Test 2: Parallel for SIMD (host-only)
    printf("\nTest 2: Parallel for SIMD\n");
    memset(c2, 0, N * sizeof(float));
    test_parallel_for_simd(a, b, c2, N);
    float checksum2 = compute_checksum(c2, N);
    printf("Checksum 2: %f\n", checksum2);
    
    // Test 3: Mixed directives
    printf("\nTest 3: Mixed directives\n");
    memset(c3, 0, N * sizeof(float));
    test_mixed_directives(a, b, c3, N);
    float checksum3 = compute_checksum(c3, N);
    printf("Checksum 3: %f\n", checksum3);
    
    // Test 4: Nested SIMD with collapse
    printf("\nTest 4: Nested SIMD\n");
    test_nested_simd(a, b, c4, N, M);
    float checksum4 = compute_checksum(c4, N * M);
    printf("Checksum 4: %f\n", checksum4);
    
    // Test 5: Complex target teams with SIMD
    printf("\nTest 5: Target teams distribute parallel for simd\n");
    if (use_gpu_offload) {
        float *c5 = (float *)aligned_alloc(64, N * sizeof(float));
        memset(c5, 0, N * sizeof(float));
        test_target_teams_simd(a, b, c5, N);
        float checksum5 = compute_checksum(c5, N);
        printf("Checksum 5: %f\n", checksum5);
        free(c5);
    }
    
    // Validation: Compare results from different paths if available
    if (use_gpu_offload) {
        printf("\nValidation: Comparing host and device results...\n");
        // Re-run test 1 on host for comparison
        float *c_host = (float *)aligned_alloc(64, N * sizeof(float));
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        test_target_simd(a, b, c_host, N);
        use_gpu_offload = saved_flag;
        
        float diff = 0.0f;
        #pragma omp simd reduction(+:diff) simdlen(4)
        for (int i = 0; i < N; i++) {
            diff += fabsf(c1[i] - c_host[i]);
        }
        printf("Difference between host and device: %e\n", diff);
        free(c_host);
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c1);
    free(c2);
    free(c3);
    free(c4);
    
    return 0;
}
