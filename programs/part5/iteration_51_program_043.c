/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT generation */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary target region for SIMT transformation */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: a[0:dynamic_N], b[0:dynamic_N], result[0:dynamic_N]) \
        private(i) shared(result) collapse(2)
    for (int i = 0; i < dynamic_N; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < dynamic_N) {
                result[idx] = a[idx] * b[idx] + (float)j;
            }
        }
    }
}

/* Alternative function with nested teams/distribute structure */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int vol_N = N;
    
    #pragma omp target teams map(tofrom: a[0:vol_N], b[0:vol_N], result[0:vol_N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a, b, result) collapse(2)
        for (int i = 0; i < vol_N; i++) {
            for (int j = 0; j < 8; j++) {
                int idx = i * 8 + j;
                if (idx < vol_N) {
                    result[idx] = a[idx] + b[idx] * (float)(i + j);
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    #pragma omp target map(tofrom: a[0:size], b[0:size], result[0:size])
    {
        #pragma omp parallel for simd private(i) shared(result)
        for (int i = 0; i < size; i++) {
            result[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* Helper function to verify results */
int verify_results(float *cpu_result, float *gpu_result, int N, float tolerance) {
    for (int i = 0; i < N; i++) {
        if (cpu_result[i] - gpu_result[i] > tolerance || 
            gpu_result[i] - cpu_result[i] > tolerance) {
            return 0;
        }
    }
    return 1;
}

/* CPU reference computation */
void cpu_compute(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < N) {
                result[idx] = a[idx] * b[idx] + (float)j;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 512;
    int N3 = (argc > 3) ? atoi(argv[3]) : 768;
    
    /* Allocate arrays with dynamic sizes */
    float *a1 = (float *)malloc(N1 * sizeof(float));
    float *b1 = (float *)malloc(N1 * sizeof(float));
    float *result1 = (float *)malloc(N1 * sizeof(float));
    float *cpu_result1 = (float *)malloc(N1 * sizeof(float));
    
    float *a2 = (float *)malloc(N2 * sizeof(float));
    float *b2 = (float *)malloc(N2 * sizeof(float));
    float *result2 = (float *)malloc(N2 * sizeof(float));
    
    float *a3 = (float *)malloc(N3 * sizeof(float));
    float *b3 = (float *)malloc(N3 * sizeof(float));
    float *result3 = (float *)malloc(N3 * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)i * 0.1f;
        b1[i] = (float)(N1 - i) * 0.05f;
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)i * 0.2f;
        b2[i] = (float)(N2 - i) * 0.1f;
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)i * 0.15f;
        b3[i] = (float)(N3 - i) * 0.075f;
    }
    
    /* Multiple invocations from different contexts */
    
    /* First: Primary SIMT transformation target */
    printf("Running primary SIMT computation...\n");
    compute_simt(N1, a1, b1, result1);
    
    /* CPU verification */
    cpu_compute(N1, a1, b1, cpu_result1);
    if (verify_results(cpu_result1, result1, N1, 0.001f)) {
        printf("Primary SIMT computation verified successfully\n");
    }
    
    /* Second: Nested teams/distribute structure */
    printf("Running nested SIMT computation...\n");
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Third: Alternative target simd construct */
    printf("Running target SIMD computation...\n");
    compute_target_simd(N3, a3, b3, result3);
    
    /* Conditional execution to expose different paths */
    if (N1 > 100) {
        printf("Running conditional SIMT computation...\n");
        compute_simt(N1/2, a1, b1, result1);
    }
    
    /* Loop with multiple invocations */
    for (int iter = 0; iter < 3; iter++) {
        printf("Iteration %d: ", iter);
        compute_simt(N1/(iter+1), a1, b1, result1);
    }
    
    /* Clean up */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    return 0;
}
