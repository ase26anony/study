#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

static int use_gpu_offload = 0;

#pragma omp declare simd uniform(a, b) linear(i)
float simd_function(float a, float b, int i) {
    return a * b + i * 0.5f;
}

void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(64) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) aligned(a, b, c: 32) \
            private(n) reduction(+:n)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_function(a[i], b[i], i);
        }
    } else {
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(8) safelen(16) \
        aligned(a, b, c: 64) schedule(static) num_threads(4)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] - i;
    }
}

void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel num_threads(2)
    {
        #pragma omp for simd collapse(2) simdlen(2) safelen(4) \
            aligned(a, b, c: 16) private(n, m)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * 2.0f + b[idx] * 3.0f + i + j;
            }
        }
    }
}

void test_mixed_directives(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for simd nowait simdlen(4)
        for (int i = 0; i < n/2; i++) {
            c[i] = a[i] / (b[i] + 1.0);
        }
        
        #pragma omp for simd simdlen(2)
        for (int i = n/2; i < n; i++) {
            c[i] = b[i] / (a[i] + 1.0);
        }
    }
}

void test_target_teams_simd(int *a, int *b, int *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(tofrom: c[0:n]) \
            simdlen(4) collapse(2) num_teams(8) thread_limit(128)
        for (int i = 0; i < n/M; i++) {
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = (a[idx] + b[idx]) * (i + j);
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

int compute_int_checksum(int *arr, int n) {
    int sum = 0;
    #pragma omp simd reduction(+:sum)
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
    
    printf("GPU offload mode: %s\n", use_gpu_offload ? "ENABLED" : "DISABLED");
    
    // Allocate and initialize arrays
    float *a_f = (float*)aligned_alloc(64, N * sizeof(float));
    float *b_f = (float*)aligned_alloc(64, N * sizeof(float));
    float *c_f = (float*)aligned_alloc(64, N * sizeof(float));
    
    int *a_i = (int*)aligned_alloc(64, N * sizeof(int));
    int *b_i = (int*)aligned_alloc(64, N * sizeof(int));
    int *c_i = (int*)aligned_alloc(64, N * sizeof(int));
    
    double *a_d = (double*)aligned_alloc(64, N * sizeof(double));
    double *b_d = (double*)aligned_alloc(64, N * sizeof(double));
    double *c_d = (double*)aligned_alloc(64, N * sizeof(double));
    
    // Initialize with patterned data
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 1.5f;
        b_f[i] = N - i * 0.5f;
        c_f[i] = 0.0f;
        
        a_i[i] = i % 100;
        b_i[i] = (N - i) % 100;
        c_i[i] = 0;
        
        a_d[i] = i * 2.0;
        b_d[i] = (N - i) * 0.5;
        c_d[i] = 0.0;
    }
    
    // Run test functions
    printf("\n=== Test 1: Target SIMD ===\n");
    test_target_simd(a_f, b_f, c_f, N);
    float checksum1 = compute_checksum(c_f, N);
    printf("Checksum 1: %f\n", checksum1);
    
    printf("\n=== Test 2: Parallel For SIMD ===\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    int checksum2 = compute_int_checksum(c_i, N);
    printf("Checksum 2: %d\n", checksum2);
    
    printf("\n=== Test 3: Nested SIMD ===\n");
    test_nested_simd(a_f, b_f, c_f, N/M, M);
    float checksum3 = compute_checksum(c_f, N);
    printf("Checksum 3: %f\n", checksum3);
    
    printf("\n=== Test 4: Mixed Directives ===\n");
    test_mixed_directives(a_d, b_d, c_d, N);
    double checksum4 = 0.0;
    #pragma omp simd reduction(+:checksum4)
    for (int i = 0; i < N; i++) {
        checksum4 += c_d[i];
    }
    printf("Checksum 4: %lf\n", checksum4);
    
    printf("\n=== Test 5: Target Teams SIMD ===\n");
    test_target_teams_simd(a_i, b_i, c_i, N);
    int checksum5 = compute_int_checksum(c_i, N);
    printf("Checksum 5: %d\n", checksum5);
    
    // Validation (if both paths were tested separately)
    if (argc > 2 && strcmp(argv[2], "--validate") == 0) {
        printf("\n=== Validation ===\n");
        // Re-run without GPU offload for comparison
        int saved_mode = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c_f_host = (float*)aligned_alloc(64, N * sizeof(float));
        memset(c_f_host, 0, N * sizeof(float));
        
        test_target_simd(a_f, b_f, c_f_host, N);
        
        int errors = 0;
        #pragma omp parallel for simd reduction(+:errors)
        for (int i = 0; i < N; i++) {
            if (fabs(c_f[i] - c_f_host[i]) > 1e-6) {
                errors++;
            }
        }
        
        printf("Validation errors: %d\n", errors);
        free(c_f_host);
        use_gpu_offload = saved_mode;
    }
    
    // Cleanup
    free(a_f); free(b_f); free(c_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    
    return 0;
}
