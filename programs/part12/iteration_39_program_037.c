#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

int use_gpu_offload = 0;

#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_function(float a, float b, int i) {
    return a * b + i * 0.5f;
}

void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(64) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(8) safelen(16) aligned(a, b, c: 64) \
            private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
            sum += c[i];
        }
        printf("GPU offload sum: %f\n", sum);
    } else {
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32) linear(i:1)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(4) safelen(8) aligned(a, b, c: 32) \
        schedule(static, 64) private(i) collapse(1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel num_threads(4) shared(a, b, c)
    {
        #pragma omp for collapse(2) private(i, j) nowait
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                #pragma omp simd simdlen(2) safelen(4) linear(idx:1)
                for (int k = 0; k < 4; k++) {
                    c[idx] += simd_function(a[idx], b[idx], k);
                }
            }
        }
    }
}

void test_mixed_directives(double *a, double *b, double *c, int n) {
    // Mixed #pragma omp for and #pragma omp simd
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            #pragma omp simd simdlen(2) aligned(a, b, c: 16)
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                c[idx] = a[idx] / (b[idx] + 1.0);
            }
        }
    }
}

void test_dynamic_simd(int *a, int *b, int *c, int n) {
    int chunk_size = 128;
    #pragma omp parallel for simd schedule(dynamic, chunk_size) \
        simdlen(8) safelen(32) private(i) reduction(max:max_val)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] - b[i];
        if (c[i] > max_val) max_val = c[i];
    }
    printf("Max value: %d\n", max_val);
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
    float *fa = (float*)aligned_alloc(64, N * sizeof(float));
    float *fb = (float*)aligned_alloc(64, N * sizeof(float));
    float *fc = (float*)aligned_alloc(64, N * sizeof(float));
    
    int *ia = (int*)aligned_alloc(32, N * sizeof(int));
    int *ib = (int*)aligned_alloc(32, N * sizeof(int));
    int *ic = (int*)aligned_alloc(32, N * sizeof(int));
    
    double *da = (double*)aligned_alloc(64, N * sizeof(double));
    double *db = (double*)aligned_alloc(64, N * sizeof(double));
    double *dc = (double*)aligned_alloc(64, N * sizeof(double));
    
    // Initialize with patterned data
    for (int i = 0; i < N; i++) {
        fa[i] = i * 0.1f;
        fb[i] = (N - i) * 0.2f;
        fc[i] = 0.0f;
        
        ia[i] = i;
        ib[i] = N - i;
        ic[i] = 0;
        
        da[i] = i * 0.01;
        db[i] = (N - i) * 0.02;
        dc[i] = 0.0;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    // Test 1: Target SIMD with conditional GPU offloading
    test_target_simd(fa, fb, fc, N);
    
    // Compute checksum
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N; i++) {
        checksum1 += fc[i];
    }
    printf("Test 1 checksum: %f\n", checksum1);
    
    // Test 2: Parallel for SIMD
    test_parallel_for_simd(ia, ib, ic, N);
    
    int checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N; i++) {
        checksum2 += ic[i];
    }
    printf("Test 2 checksum: %d\n", checksum2);
    
    // Test 3: Nested SIMD
    test_nested_simd(fa, fb, fc, N/4, 4);
    
    float checksum3 = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum3 += fc[i];
    }
    printf("Test 3 checksum: %f\n", checksum3);
    
    // Test 4: Mixed directives
    test_mixed_directives(da, db, dc, N/4);
    
    double checksum4 = 0.0;
    #pragma omp simd reduction(+:checksum4)
    for (int i = 0; i < N; i++) {
        checksum4 += dc[i];
    }
    printf("Test 4 checksum: %f\n", checksum4);
    
    // Test 5: Dynamic SIMD
    test_dynamic_simd(ia, ib, ic, N);
    
    // Cleanup
    free(fa); free(fb); free(fc);
    free(ia); free(ib); free(ic);
    free(da); free(db); free(dc);
    
    return 0;
}
