/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT generation */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT candidate construct */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) \
        num_teams(256) thread_limit(128)
    for (int i = 0; i < dynamic_N; i++) {
        result[i] = a[i] * b[i] + (float)i / N;
    }
}

/* Alternative function with nested teams/distribute */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int rows = N / 16;
    volatile int cols = 16;
    
    #pragma omp target teams map(to: a[0:N], b[0:N]) map(from: result[0:N]) \
        num_teams(128)
    {
        #pragma omp distribute parallel for simd collapse(2) \
            private(i,j) shared(a,b,result)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                if (idx < N) {
                    result[idx] = a[idx] * 2.0f + b[idx] * 3.0f;
                }
            }
        }
    }
}

/* Function with target simd that might trigger parallel SIMT */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    #pragma omp target simd map(to: a[0:size], b[0:size]) \
        map(from: result[0:size]) linear(i:1)
    for (int i = 0; i < size; i++) {
        result[i] = a[i] + b[i] * (i % 8);
    }
}

/* Helper function to verify results */
int verify_results(int N, float *cpu_result, float *gpu_result, float tolerance) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float diff = cpu_result[i] - gpu_result[i];
        if (diff < -tolerance || diff > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                       i, cpu_result[i], gpu_result[i]);
            }
        }
    }
    return errors;
}

/* CPU reference implementation */
void cpu_compute(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        result[i] = a[i] * b[i] + (float)i / N;
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant size from command line or environment */
    int N = 1024;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1024;
    }
    
    /* Dynamic allocation to prevent compile-time optimization */
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *gpu_result1 = (float *)malloc(N * sizeof(float));
    float *gpu_result2 = (float *)malloc(N * sizeof(float));
    float *cpu_result = (float *)malloc(N * sizeof(float));
    
    if (!a || !b || !gpu_result1 || !gpu_result2 || !cpu_result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    srand(42);
    for (int i = 0; i < N; i++) {
        a[i] = (float)rand() / RAND_MAX;
        b[i] = (float)rand() / RAND_MAX;
    }
    
    printf("Testing SIMT transformation with N = %d\n", N);
    
    /* Test 1: Primary SIMT construct - called multiple times */
    for (int iter = 0; iter < 3; iter++) {
        compute_simt(N, a, b, gpu_result1);
    }
    
    /* Test 2: Nested construct in conditional context */
    if (N > 512) {
        compute_nested_simt(N, a, b, gpu_result2);
    } else {
        /* Alternative path with different construct */
        compute_target_simd(N, a, b, gpu_result2);
    }
    
    /* Test 3: Call from different control flow point */
    {
        volatile int flag = 1;
        if (flag) {
            compute_simt(N / 2, a, b, gpu_result1);
        }
    }
    
    /* CPU reference computation */
    cpu_compute(N, a, b, cpu_result);
    
    /* Verify results */
    int errors1 = verify_results(N, cpu_result, gpu_result1, 1e-5f);
    int errors2 = verify_results(N, cpu_result, gpu_result2, 1e-5f);
    
    printf("Verification results:\n");
    printf("  GPU result 1: %d errors\n", errors1);
    printf("  GPU result 2: %d errors\n", errors2);
    
    /* Additional test with collapse clause */
    if (N >= 64) {
        float *gpu_result3 = (float *)malloc(N * sizeof(float));
        volatile int dim1 = 32;
        volatile int dim2 = N / 32;
        
        #pragma omp target teams distribute parallel for simd collapse(2) \
            map(to: a[0:N], b[0:N]) map(from: gpu_result3[0:N]) \
            private(i,j,idx) shared(a,b,gpu_result3)
        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim2; j++) {
                int idx = i * dim2 + j;
                if (idx < N) {
                    gpu_result3[idx] = a[idx] * b[idx] * (i + 1);
                }
            }
        }
        
        free(gpu_result3);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(gpu_result1);
    free(gpu_result2);
    free(cpu_result);
    
    printf("Test completed\n");
    return 0;
}
