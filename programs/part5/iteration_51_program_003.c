/* Test program to trigger SIMT transformation in GCC's OpenMP lowering */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Function containing primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) collapse(2)
    for (int i = 0; i < dynamic_N; i++) {
        for (int j = 0; j < 16; j++) {
            result[i] = a[i] * b[i] + (float)j * 0.1f;
        }
    }
}

/* Alternative function with nested teams/distribute */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    #pragma omp target teams map(to: a[0:size], b[0:size]) map(from: result[0:size])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a, b, result) collapse(2)
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < 8; j++) {
                result[i] = a[i] + b[i] * (float)j;
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int n = N;
    
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: result[0:n])
    {
        #pragma omp parallel
        {
            #pragma omp simd private(i)
            for (int i = 0; i < n; i++) {
                result[i] = sqrtf(a[i] * a[i] + b[i] * b[i]);
            }
        }
    }
}

/* Helper function to verify results */
int verify_results(float *cpu, float *gpu, int N, float tolerance) {
    for (int i = 0; i < N; i++) {
        if (fabsf(cpu[i] - gpu[i]) > tolerance) {
            printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                   i, cpu[i], gpu[i]);
            return 0;
        }
    }
    return 1;
}

/* CPU reference implementation */
void cpu_compute(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            result[i] = a[i] * b[i] + (float)j * 0.1f;
        }
    }
}

int main(int argc, char **argv) {
    /* Use non-constant sizes from command line or defaults */
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
    
    /* Multiple invocations with different constructs */
    
    /* First: Primary SIMT transformation target */
    printf("Running primary SIMT computation...\n");
    compute_simt(N1, a1, b1, result1);
    
    /* CPU verification */
    cpu_compute(N1, a1, b1, cpu_result1);
    if (verify_results(cpu_result1, result1, N1, 1e-5f)) {
        printf("Primary SIMT computation verified successfully\n");
    }
    
    /* Second: Nested teams/distribute pattern */
    printf("Running nested teams/distribute computation...\n");
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Third: target simd with parallel region */
    printf("Running target simd with parallel region...\n");
    compute_target_simd(N3, a3, b3, result3);
    
    /* Additional call with conditional execution path */
    if (N1 > 100) {
        printf("Running conditional SIMT computation...\n");
        compute_simt(N1/2, a1, b1, result1);
    }
    
    /* Alternative construct in dead code (exposed to parser but not executed) */
    if (0) {
        /* This exposes the compiler to another variant without executing it */
        #pragma omp target teams distribute parallel for simd \
            map(to: a1[0:N1]) map(from: result1[0:N1]) \
            private(i) collapse(3)
        for (int i = 0; i < N1; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    result1[i] += a1[i] * (float)(j * k);
                }
            }
        }
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    printf("All computations completed\n");
    return 0;
}
