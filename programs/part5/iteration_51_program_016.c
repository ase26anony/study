/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary target region for SIMT transformation */
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
            result[i] = a[i] * b[i] + j * 0.1f;
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
            simdlen(8)
        for (int i = 0; i < dyn_N; i++) {
            result[i] = a[i] * 2.0f + b[i] * 3.0f;
        }
    }
}

/* Function with target simd and parallel execution hint */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int v_N = N;
    
    #pragma omp target map(to: a[0:v_N], b[0:v_N]) map(from: result[0:v_N])
    #pragma omp parallel for simd
    for (int i = 0; i < v_N; i++) {
        result[i] = a[i] + b[i];
    }
}

/* Helper function to verify results */
int verify_results(int N, float *cpu_result, float *gpu_result) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(cpu_result[i] - gpu_result[i]) > 1e-6f) {
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
        for (int j = 0; j < 10; j++) {
            result[i] = a[i] * b[i] + j * 0.1f;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 512;
    int N3 = (argc > 3) ? atoi(argv[3]) : 768;
    
    /* Dynamic allocation to avoid constant propagation */
    float *a1 = (float *)malloc(N1 * sizeof(float));
    float *b1 = (float *)malloc(N1 * sizeof(float));
    float *result1_gpu = (float *)malloc(N1 * sizeof(float));
    float *result1_cpu = (float *)malloc(N1 * sizeof(float));
    
    float *a2 = (float *)malloc(N2 * sizeof(float));
    float *b2 = (float *)malloc(N2 * sizeof(float));
    float *result2 = (float *)malloc(N2 * sizeof(float));
    
    float *a3 = (float *)malloc(N3 * sizeof(float));
    float *b3 = (float *)malloc(N3 * sizeof(float));
    float *result3 = (float *)malloc(N3 * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)i * 1.5f;
        b1[i] = (float)i * 2.5f;
    }
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)i * 0.7f;
        b2[i] = (float)i * 1.3f;
    }
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)i * 2.0f;
        b3[i] = (float)i * 3.0f;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Primary SIMT construct with collapse clause */
    printf("Test 1: target teams distribute parallel for simd with collapse(2)\n");
    compute_simt(N1, a1, b1, result1_gpu);
    
    /* CPU verification */
    cpu_compute(N1, a1, b1, result1_cpu);
    int errors1 = verify_results(N1, result1_cpu, result1_gpu);
    printf("Test 1 errors: %d\n", errors1);
    
    /* Test 2: Nested teams/distribute construct */
    printf("\nTest 2: Nested teams with distribute parallel for simd\n");
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Test 3: target simd with parallel hint */
    printf("\nTest 3: target with parallel for simd\n");
    compute_target_simd(N3, a3, b3, result3);
    
    /* Additional test with conditional execution to affect context */
    printf("\nTest 4: Conditional execution paths\n");
    for (int iter = 0; iter < 3; iter++) {
        if (iter % 2 == 0) {
            compute_simt(N1, a1, b1, result1_gpu);
        } else {
            compute_nested_simt(N2, a2, b2, result2);
        }
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1_gpu); free(result1_cpu);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    printf("\nAll tests completed.\n");
    return 0;
}
