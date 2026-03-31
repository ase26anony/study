/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT generation */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) \
        collapse(2)
    for (int i = 0; i < dynamic_N; i++) {
        for (int j = 0; j < 10; j++) {
            result[i] = a[i] * b[i] + (float)j;
        }
    }
}

/* Alternative function with nested teams/distribute construct */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int dyn_N = N;
    
    #pragma omp target teams map(to: a[0:dyn_N], b[0:dyn_N]) map(from: result[0:dyn_N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(result) \
            collapse(2)
        for (int i = 0; i < dyn_N; i++) {
            for (int j = 0; j < 8; j++) {
                result[i] = a[i] + b[i] * (float)j;
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int v_N = N;
    
    /* Create context that might trigger parallel SIMT transformation */
    #pragma omp target map(to: a[0:v_N], b[0:v_N]) map(from: result[0:v_N])
    #pragma omp parallel
    #pragma omp simd
    for (int i = 0; i < v_N; i++) {
        result[i] = a[i] - b[i];
    }
}

/* Helper function to verify results */
int verify_results(float *cpu_result, float *gpu_result, int N) {
    for (int i = 0; i < N; i++) {
        if (cpu_result[i] != gpu_result[i]) {
            printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                   i, cpu_result[i], gpu_result[i]);
            return 0;
        }
    }
    return 1;
}

/* CPU reference computation */
void cpu_compute(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 10; j++) {
            result[i] = a[i] * b[i] + (float)j;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 512;
    
    /* Dynamic allocation prevents constant propagation */
    float *a1 = (float *)malloc(N1 * sizeof(float));
    float *b1 = (float *)malloc(N1 * sizeof(float));
    float *result1_gpu = (float *)malloc(N1 * sizeof(float));
    float *result1_cpu = (float *)malloc(N1 * sizeof(float));
    
    float *a2 = (float *)malloc(N2 * sizeof(float));
    float *b2 = (float *)malloc(N2 * sizeof(float));
    float *result2_gpu = (float *)malloc(N2 * sizeof(float));
    float *result2_cpu = (float *)malloc(N2 * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)i;
        b1[i] = (float)(i * 2);
        result1_gpu[i] = 0.0f;
        result1_cpu[i] = 0.0f;
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)(i + 1);
        b2[i] = (float)(i * 3);
        result2_gpu[i] = 0.0f;
        result2_cpu[i] = 0.0f;
    }
    
    /* Multiple invocations with different contexts */
    
    /* First call - primary SIMT construct */
    compute_simt(N1, a1, b1, result1_gpu);
    
    /* CPU reference */
    cpu_compute(N1, a1, b1, result1_cpu);
    
    /* Second call - nested construct */
    compute_nested_simt(N2, a2, b2, result2_gpu);
    
    /* Modified CPU computation for second case */
    for (int i = 0; i < N2; i++) {
        for (int j = 0; j < 8; j++) {
            result2_cpu[i] = a2[i] + b2[i] * (float)j;
        }
    }
    
    /* Third call - different function, same data */
    if (N1 > 100) {
        compute_target_simd(N1, a1, b1, result1_gpu);
    }
    
    /* Verify results */
    int success1 = verify_results(result1_cpu, result1_gpu, N1);
    int success2 = verify_results(result2_cpu, result2_gpu, N2);
    
    printf("Test 1: %s\n", success1 ? "PASS" : "FAIL");
    printf("Test 2: %s\n", success2 ? "PASS" : "FAIL");
    
    /* Additional test case in conditional block */
    if (0) {  /* Prevent execution but expose to compiler */
        /* Alternative syntax that should trigger same transformation */
        #pragma omp target teams distribute simd \
            map(tofrom: a1[0:N1]) \
            private(i)
        for (int i = 0; i < N1; i++) {
            a1[i] *= 2.0f;
        }
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1_gpu); free(result1_cpu);
    free(a2); free(b2); free(result2_gpu); free(result2_cpu);
    
    return (success1 && success2) ? 0 : 1;
}
