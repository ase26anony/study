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
        for (int j = 0; j < 16; j++) {  /* Inner loop for collapse(2) */
            int idx = i * 16 + j;
            if (idx < dynamic_N) {
                result[idx] = a[idx] * b[idx] + (float)j;
            }
        }
    }
}

/* Alternative function with nested teams/distribute */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int dyn_N = N;
    
    #pragma omp target teams map(to: a[0:dyn_N], b[0:dyn_N]) map(from: result[0:dyn_N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(result) \
            simdlen(8)
        for (int i = 0; i < dyn_N; i++) {
            result[i] = a[i] + b[i] * 2.0f;
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int vN = N;
    
    /* Create context that might trigger parallel SIMT transformation */
    #pragma omp target map(to: a[0:vN], b[0:vN]) map(from: result[0:vN])
    #pragma omp parallel
    {
        #pragma omp for simd nowait
        for (int i = 0; i < vN; i++) {
            result[i] = a[i] - b[i];
        }
    }
}

/* Verification function */
int verify_results(int N, float *cpu_result, float *gpu_result, float tolerance) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float diff = cpu_result[i] - gpu_result[i];
        if (diff < -tolerance || diff > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at %d: CPU=%f, GPU=%f\n", 
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
            int idx = i * 16 + j;
            if (idx < N) {
                result[idx] = a[idx] * b[idx] + (float)j;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes from command line or defaults */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 512;
    int N3 = (argc > 3) ? atoi(argv[3]) : 768;
    
    /* Allocate arrays with dynamic sizes */
    float *a1 = (float*)malloc(N1 * sizeof(float));
    float *b1 = (float*)malloc(N1 * sizeof(float));
    float *result1_gpu = (float*)malloc(N1 * sizeof(float));
    float *result1_cpu = (float*)malloc(N1 * sizeof(float));
    
    float *a2 = (float*)malloc(N2 * sizeof(float));
    float *b2 = (float*)malloc(N2 * sizeof(float));
    float *result2_gpu = (float*)malloc(N2 * sizeof(float));
    
    float *a3 = (float*)malloc(N3 * sizeof(float));
    float *b3 = (float*)malloc(N3 * sizeof(float));
    float *result3_gpu = (float*)malloc(N3 * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)i * 0.1f;
        b1[i] = (float)(N1 - i) * 0.05f;
    }
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)i * 0.2f;
        b2[i] = (float)i * 0.1f;
    }
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)(i % 100) * 0.3f;
        b3[i] = (float)(i / 10) * 0.15f;
    }
    
    /* Multiple invocations to hit different contexts */
    printf("Test 1: Primary SIMT transformation...\n");
    compute_simt(N1, a1, b1, result1_gpu);
    
    printf("Test 2: Nested teams/distribute...\n");
    compute_nested_simt(N2, a2, b2, result2_gpu);
    
    printf("Test 3: Target simd with parallel region...\n");
    compute_target_simd(N3, a3, b3, result3_gpu);
    
    /* Conditional block with alternative constructs */
    if (argc > 4) {
        /* This block exposes compiler to different syntax */
        printf("Test 4: Conditional execution path...\n");
        float *temp = (float*)malloc(N1 * sizeof(float));
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: temp[0:N1]) \
            private(i)
        for (int i = 0; i < N1; i++) {
            temp[i] = a1[i] * 2.0f;
        }
        free(temp);
    }
    
    /* Verification */
    cpu_compute(N1, a1, b1, result1_cpu);
    int errors = verify_results(N1, result1_cpu, result1_gpu, 0.001f);
    
    if (errors == 0) {
        printf("All tests passed!\n");
    } else {
        printf("Found %d errors in verification\n", errors);
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1_gpu); free(result1_cpu);
    free(a2); free(b2); free(result2_gpu);
    free(a3); free(b3); free(result3_gpu);
    
    return (errors > 0) ? 1 : 0;
}
