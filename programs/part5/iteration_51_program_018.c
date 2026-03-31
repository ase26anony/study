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
        private(i) shared(result) collapse(2)
    for (int i = 0; i < dynamic_N; i++) {
        for (int j = 0; j < 16; j++) {  /* Inner loop for collapse(2) */
            result[i] = a[i] * b[i] + (float)j * 0.1f;
        }
    }
}

/* Alternative function with nested teams/distribute structure */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int dyn_N = N;
    
    #pragma omp target teams map(to: a[0:dyn_N], b[0:dyn_N]) map(from: result[0:dyn_N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(result) collapse(2)
        for (int i = 0; i < dyn_N; i++) {
            for (int j = 0; j < 8; j++) {
                result[i] = a[i] + b[i] * (float)j;
            }
        }
    }
}

/* Function with target simd that might trigger parallel SIMT */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int vN = N;
    
    #pragma omp target simd map(to: a[0:vN], b[0:vN]) map(from: result[0:vN]) \
        private(i) linear(j:1)
    for (int i = 0; i < vN; i++) {
        for (int j = 0; j < 4; j++) {
            result[i] = a[i] * 2.0f + b[i] * (float)j;
        }
    }
}

/* Helper function to verify results */
int verify_results(int N, float *cpu_result, float *gpu_result) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(cpu_result[i] - gpu_result[i]) > 1e-6) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                       i, cpu_result[i], gpu_result[i]);
            }
        }
    }
    return errors;
}

/* CPU reference computation */
void cpu_compute(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            result[i] = a[i] * b[i] + (float)j * 0.1f;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes from command line or defaults */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 512;
    int N3 = (argc > 3) ? atoi(argv[3]) : 768;
    
    /* Dynamic allocation to prevent compile-time optimization */
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
    
    /* First: Primary SIMT transformation */
    printf("Running primary SIMT computation (N=%d)...\n", N1);
    compute_simt(N1, a1, b1, result1);
    
    /* CPU verification */
    cpu_compute(N1, a1, b1, cpu_result1);
    int errors1 = verify_results(N1, cpu_result1, result1);
    printf("Primary SIMT errors: %d\n", errors1);
    
    /* Second: Nested structure from conditional context */
    if (N2 > 256) {  /* Conditional to create different control flow */
        printf("Running nested SIMT computation (N=%d)...\n", N2);
        compute_nested_simt(N2, a2, b2, result2);
    }
    
    /* Third: Alternative construct in loop context */
    for (int iter = 0; iter < 2; iter++) {
        if (iter == 1) {  /* Second iteration uses different construct */
            printf("Running target SIMD computation (N=%d, iter=%d)...\n", N3, iter);
            compute_target_simd(N3, a3, b3, result3);
        }
    }
    
    /* Additional call with different size to hit different paths */
    if (argc > 4) {
        int N4 = atoi(argv[4]);
        float *a4 = (float *)malloc(N4 * sizeof(float));
        float *b4 = (float *)malloc(N4 * sizeof(float));
        float *result4 = (float *)malloc(N4 * sizeof(float));
        
        for (int i = 0; i < N4; i++) {
            a4[i] = (float)i * 0.25f;
            b4[i] = (float)(N4 - i) * 0.125f;
        }
        
        compute_simt(N4, a4, b4, result4);
        free(a4); free(b4); free(result4);
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    printf("Test completed.\n");
    return 0;
}
