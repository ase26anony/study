/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT generation */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    /* Primary target region designed to trigger SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) collapse(2)
    for (int i = 0; i < dynamic_N; i++) {
        for (int j = 0; j < 16; j++) {  /* Inner loop for collapse(2) */
            result[i] = a[i] * b[i] + (float)j * 0.1f;
        }
    }
}

/* Alternative function with nested teams/distribute construct */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int vol_N = N;
    
    /* Nested construct that should also trigger SIMT transformation */
    #pragma omp target teams map(tofrom: a[0:vol_N], b[0:vol_N], result[0:vol_N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a, b, result) \
            collapse(2)
        for (int i = 0; i < vol_N; i++) {
            for (int j = 0; j < 8; j++) {
                result[i] = a[i] + b[i] * (float)j;
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    /* target simd in a context suggesting parallel GPU execution */
    #pragma omp target map(to: a[0:size], b[0:size]) map(from: result[0:size])
    #pragma omp parallel
    {
        #pragma omp simd
        for (int i = 0; i < size; i++) {
            result[i] = a[i] - b[i];
        }
    }
}

/* Verification function to prevent dead code elimination */
int verify_results(float *cpu, float *gpu, int N, float tolerance) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabsf(cpu[i] - gpu[i]) > tolerance) {
            errors++;
        }
    }
    return errors;
}

/* CPU reference implementation */
void cpu_compute(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            result[i] = a[i] * b[i] + (float)j * 0.1f;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int base_size = 1024;
    if (argc > 1) base_size = atoi(argv[1]);
    
    /* Dynamic allocation prevents constant propagation */
    int N1 = base_size;
    int N2 = base_size * 2;
    int N3 = base_size / 2;
    
    float *a1 = (float*)malloc(N1 * sizeof(float));
    float *b1 = (float*)malloc(N1 * sizeof(float));
    float *result1_gpu = (float*)malloc(N1 * sizeof(float));
    float *result1_cpu = (float*)malloc(N1 * sizeof(float));
    
    float *a2 = (float*)malloc(N2 * sizeof(float));
    float *b2 = (float*)malloc(N2 * sizeof(float));
    float *result2 = (float*)malloc(N2 * sizeof(float));
    
    float *a3 = (float*)malloc(N3 * sizeof(float));
    float *b3 = (float*)malloc(N3 * sizeof(float));
    float *result3 = (float*)malloc(N3 * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)i * 0.5f;
        b1[i] = (float)i * 1.5f;
    }
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)i * 0.3f;
        b2[i] = (float)i * 0.7f;
    }
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)i * 1.1f;
        b3[i] = (float)i * 2.2f;
    }
    
    /* Multiple invocations to hit different contexts */
    
    /* First invocation - primary SIMT pattern */
    compute_simt(N1, a1, b1, result1_gpu);
    
    /* CPU reference for verification */
    cpu_compute(N1, a1, b1, result1_cpu);
    
    /* Second invocation - nested pattern */
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Third invocation - different control flow path */
    if (N3 > 0) {
        compute_target_simd(N3, a3, b3, result3);
    }
    
    /* Additional invocation with different size */
    compute_simt(N3, a3, b3, result3);
    
    /* Verification to prevent elimination */
    int errors = verify_results(result1_cpu, result1_gpu, N1, 0.001f);
    
    /* Use results to prevent dead code elimination */
    float sum = 0.0f;
    for (int i = 0; i < N1; i++) sum += result1_gpu[i];
    for (int i = 0; i < N2; i++) sum += result2[i];
    for (int i = 0; i < N3; i++) sum += result3[i];
    
    printf("Verification errors: %d\n", errors);
    printf("Result sum: %f\n", sum);
    
    /* Cleanup */
    free(a1); free(b1); free(result1_gpu); free(result1_cpu);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    return errors > 0 ? 1 : 0;
}
