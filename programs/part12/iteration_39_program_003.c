#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

static int use_gpu_offload = 0;

#pragma omp declare simd uniform(a, b) linear(i:1) simdlen(4)
float simd_function(float a, float b, int i) {
    return a * b + i * 0.5f;
}

void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(64) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(8) safelen(16) aligned(a, b, c: 32) \
            private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            float temp = simd_function(a[i], b[i], i);
            c[i] = temp + a[i] * 0.3f;
        }
    } else {
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 16) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

void test_parallel_for_simd(float *a, float *b, float *c, int n) {
    #pragma omp parallel for simd simdlen(4) safelen(8) \
        aligned(a, b, c: 32) schedule(static, 64) \
        private(i) if(n > 100)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] - i * 0.1f;
    }
}

void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel num_threads(4) shared(a, b, c) private(i, j)
    {
        #pragma omp for collapse(2) simd simdlen(4) safelen(8) \
            aligned(a, b, c: 16) schedule(static)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * 2.0f + b[idx] * 3.0f + i + j * 0.5f;
            }
        }
    }
}

void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for simd simdlen(8) nowait
        for (int i = 0; i < n/2; i++) {
            c[i] = a[i] - b[i];
        }
        
        #pragma omp for simd simdlen(4) ordered
        for (int i = n/2; i < n; i++) {
            c[i] = b[i] - a[i];
        }
    }
}

void test_target_teams_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(tofrom: c[0:n]) \
        num_teams(8) num_threads(32) \
        simdlen(4) collapse(2) \
        private(i, j) if(target: use_gpu_offload)
    for (int i = 0; i < n/M; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c[idx] = a[idx] * b[idx] / (idx + 1.0f);
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
    // Parse command line argument for GPU offloading
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    // Allocate and initialize arrays
    float *a = (float *)aligned_alloc(32, N * M * sizeof(float));
    float *b = (float *)aligned_alloc(32, N * M * sizeof(float));
    float *c = (float *)aligned_alloc(32, N * M * sizeof(float));
    float *d = (float *)aligned_alloc(32, N * M * sizeof(float));
    
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < N * M; i++) {
        a[i] = (float)i;
        b[i] = (float)(N * M - i);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    // Test 1: Target SIMD with conditional offloading
    double start = omp_get_wtime();
    test_target_simd(a, b, c, N * M);
    double end = omp_get_wtime();
    float checksum1 = compute_checksum(c, N * M);
    printf("Test 1 checksum: %f, Time: %f sec\n", checksum1, end - start);
    
    // Test 2: Parallel for SIMD
    start = omp_get_wtime();
    test_parallel_for_simd(a, b, d, N * M);
    end = omp_get_wtime();
    float checksum2 = compute_checksum(d, N * M);
    printf("Test 2 checksum: %f, Time: %f sec\n", checksum2, end - start);
    
    // Test 3: Nested SIMD
    #pragma omp simd
    for (int i = 0; i < N * M; i++) c[i] = 0.0f;
    
    start = omp_get_wtime();
    test_nested_simd(a, b, c, N, M);
    end = omp_get_wtime();
    float checksum3 = compute_checksum(c, N * M);
    printf("Test 3 checksum: %f, Time: %f sec\n", checksum3, end - start);
    
    // Test 4: Mixed directives
    #pragma omp simd
    for (int i = 0; i < N * M; i++) d[i] = 0.0f;
    
    start = omp_get_wtime();
    test_mixed_directives(a, b, d, N * M);
    end = omp_get_wtime();
    float checksum4 = compute_checksum(d, N * M);
    printf("Test 4 checksum: %f, Time: %f sec\n", checksum4, end - start);
    
    // Test 5: Target teams with SIMD
    if (use_gpu_offload) {
        #pragma omp simd
        for (int i = 0; i < N * M; i++) c[i] = 0.0f;
        
        start = omp_get_wtime();
        test_target_teams_simd(a, b, c, N * M);
        end = omp_get_wtime();
        float checksum5 = compute_checksum(c, N * M);
        printf("Test 5 checksum: %f, Time: %f sec\n", checksum5, end - start);
    }
    
    // Validation: Compare host and device results if both paths taken
    if (use_gpu_offload) {
        float *host_result = (float *)malloc(N * M * sizeof(float));
        float *device_result = (float *)malloc(N * M * sizeof(float));
        
        // Run host version
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        test_target_simd(a, b, host_result, N * M);
        use_gpu_offload = saved_flag;
        
        // Run device version
        test_target_simd(a, b, device_result, N * M);
        
        // Compare
        float diff = 0.0f;
        #pragma omp simd reduction(+:diff) simdlen(4)
        for (int i = 0; i < N * M; i++) {
            diff += fabsf(host_result[i] - device_result[i]);
        }
        
        printf("Validation - Host vs Device difference: %e\n", diff);
        
        free(host_result);
        free(device_result);
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
