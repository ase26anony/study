#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

/* Global flag to control GPU offloading */
static int use_gpu_offload = 0;

/* Function declared with SIMD attribute */
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_multiply(float a, float b, int i) {
    return a * b * (i % 10);
}

/* Test 1: Target SIMD with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        num_teams(4) thread_limit(128) \
        simdlen(4) safelen(8) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        private(n) reduction(+:n)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] + simd_multiply(a[i], b[i], i);
    }
}

/* Test 2: Parallel for SIMD with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c: 64) \
        linear(i:1) private(i) collapse(1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * 2 + b[i] / 3;
    }
}

/* Test 3: Nested SIMD with collapse */
void test_nested_simd(double *a, double *b, double *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) \
            simdlen(4) safelen(8) \
            private(i, j) lastprivate(j)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * b[idx] + (i + j) * 0.5;
            }
        }
    }
}

/* Test 4: Teams distribute with SIMD - complex nesting */
void test_teams_distribute_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(tofrom: c[0:n]) \
        simdlen(4) collapse(2) \
        num_teams(n/64) thread_limit(256)
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            c[idx] = (a[idx] - b[idx]) * (a[idx] + b[idx]);
        }
    }
}

/* Test 5: Mixed directives - SIMD inside parallel region */
void test_mixed_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            #pragma omp simd simdlen(4) aligned(a, b, c: 32)
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] | b[idx] & 0xFF;
            }
        }
    }
}

/* Helper function to compute checksum */
long long compute_checksum(void *data, size_t size) {
    long long sum = 0;
    unsigned char *bytes = (unsigned char *)data;
    for (size_t i = 0; i < size; i++) {
        sum += bytes[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    /* Parse command line argument for GPU offloading */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    /* Allocate and initialize test arrays */
    float *fa = (float *)malloc(N * sizeof(float));
    float *fb = (float *)malloc(N * sizeof(float));
    float *fc = (float *)malloc(N * sizeof(float));
    
    int *ia = (int *)malloc(N * M * sizeof(int));
    int *ib = (int *)malloc(N * M * sizeof(int));
    int *ic = (int *)malloc(N * M * sizeof(int));
    
    double *da = (double *)malloc(N * M * sizeof(double));
    double *db = (double *)malloc(N * M * sizeof(double));
    double *dc = (double *)malloc(N * M * sizeof(double));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; i++) {
        fa[i] = i * 1.5f;
        fb[i] = N - i * 0.7f;
        fc[i] = 0.0f;
    }
    
    for (int i = 0; i < N * M; i++) {
        ia[i] = i % 256;
        ib[i] = (i * 3) % 256;
        ic[i] = 0;
        da[i] = i * 0.25;
        db[i] = i * 0.75;
        dc[i] = 0.0;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    /* Test 1: Target SIMD with conditional execution */
    printf("\nTest 1: Target SIMD\n");
    test_target_simd(fa, fb, fc, N);
    long long checksum1 = compute_checksum(fc, N * sizeof(float));
    printf("Checksum 1: %lld\n", checksum1);
    
    /* Test 2: Parallel for SIMD */
    printf("\nTest 2: Parallel for SIMD\n");
    test_parallel_for_simd(ia, ib, ic, N * M);
    long long checksum2 = compute_checksum(ic, N * M * sizeof(int));
    printf("Checksum 2: %lld\n", checksum2);
    
    /* Test 3: Nested SIMD */
    printf("\nTest 3: Nested SIMD\n");
    test_nested_simd(da, db, dc, N, M);
    long long checksum3 = compute_checksum(dc, N * M * sizeof(double));
    printf("Checksum 3: %lld\n", checksum3);
    
    /* Test 4: Teams distribute SIMD */
    printf("\nTest 4: Teams distribute SIMD\n");
    test_teams_distribute_simd(fa, fb, fc, N);
    long long checksum4 = compute_checksum(fc, N * sizeof(float));
    printf("Checksum 4: %lld\n", checksum4);
    
    /* Test 5: Mixed directives */
    printf("\nTest 5: Mixed directives\n");
    test_mixed_simd(ia, ib, ic, N);
    long long checksum5 = compute_checksum(ic, N * M * sizeof(int));
    printf("Checksum 5: %lld\n", checksum5);
    
    /* Run host-only version for comparison if GPU was used */
    if (use_gpu_offload) {
        printf("\nRunning host-only comparison...\n");
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        /* Clear output arrays */
        memset(fc, 0, N * sizeof(float));
        memset(ic, 0, N * M * sizeof(int));
        memset(dc, 0, N * M * sizeof(double));
        
        /* Re-run tests without GPU offloading */
        test_target_simd(fa, fb, fc, N);
        test_parallel_for_simd(ia, ib, ic, N * M);
        test_nested_simd(da, db, dc, N, M);
        
        long long host_checksum1 = compute_checksum(fc, N * sizeof(float));
        long long host_checksum2 = compute_checksum(ic, N * M * sizeof(int));
        long long host_checksum3 = compute_checksum(dc, N * M * sizeof(double));
        
        printf("Host checksums: %lld, %lld, %lld\n", 
               host_checksum1, host_checksum2, host_checksum3);
        
        use_gpu_offload = saved_flag;
    }
    
    /* Cleanup */
    free(fa); free(fb); free(fc);
    free(ia); free(ib); free(ic);
    free(da); free(db); free(dc);
    
    printf("\nAll tests completed.\n");
    return 0;
}
