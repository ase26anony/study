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

/* Function with target simd that might trigger parallel SIMT transformation */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int v_N = N;
    
    #pragma omp target map(to: a[0:v_N], b[0:v_N]) map(from: result[0:v_N])
    #pragma omp parallel for simd private(i) shared(result)
    for (int i = 0; i < v_N; i++) {
        result[i] = a[i] - b[i];
    }
}

/* Helper function to verify results */
int verify_results(float *cpu_result, float *gpu_result, int N) {
    for (int i = 0; i < N; i++) {
        if (fabs(cpu_result[i] - gpu_result[i]) > 1e-6f) {
            printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                   i, cpu_result[i], gpu_result[i]);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
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
        a1[i] = (float)i * 1.5f;
        b1[i] = (float)i * 0.5f;
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)i * 2.0f;
        b2[i] = (float)i * 0.75f;
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)i * 1.2f;
        b3[i] = (float)i * 0.3f;
    }
    
    /* Multiple invocations to hit different contexts */
    printf("Running SIMT computation 1...\n");
    compute_simt(N1, a1, b1, result1);
    
    /* Compute CPU reference for verification */
    for (int i = 0; i < N1; i++) {
        float sum = 0.0f;
        for (int j = 0; j < 16; j++) {
            sum += a1[i] * b1[i] + (float)j * 0.1f;
        }
        cpu_result1[i] = sum / 16.0f;
    }
    
    printf("Running SIMT computation 2...\n");
    compute_nested_simt(N2, a2, b2, result2);
    
    printf("Running SIMT computation 3...\n");
    compute_target_simd(N3, a3, b3, result3);
    
    /* Conditional block with alternative constructs */
    if (argc > 4) {
        /* This block contains alternative OpenMP forms that should
           also trigger SIMT transformation but might be optimized
           differently due to conditional execution */
        volatile int alt_N = 256;
        float *alt_a = (float *)malloc(alt_N * sizeof(float));
        float *alt_b = (float *)malloc(alt_N * sizeof(float));
        float *alt_result = (float *)malloc(alt_N * sizeof(float));
        
        #pragma omp target teams distribute parallel for simd \
            map(to: alt_a[0:alt_N], alt_b[0:alt_N]) \
            map(from: alt_result[0:alt_N]) \
            private(i) collapse(2)
        for (int i = 0; i < alt_N; i++) {
            for (int j = 0; j < 4; j++) {
                alt_result[i] = alt_a[i] * alt_b[i] * (float)j;
            }
        }
        
        free(alt_a);
        free(alt_b);
        free(alt_result);
    }
    
    /* Verify results */
    if (verify_results(cpu_result1, result1, N1)) {
        printf("Computation 1 passed verification\n");
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    return 0;
}
