/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT generation */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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
    
    #pragma omp target teams map(to: a[0:dyn_N], b[0:dyn_N]) \
                             map(from: result[0:dyn_N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a,b,result) collapse(2)
        for (int i = 0; i < dyn_N; i++) {
            for (int j = 0; j < 8; j++) {
                result[i] = sinf(a[i]) * cosf(b[i]) + (float)j * 0.05f;
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int vN = N;
    
    /* Create context that suggests parallel execution */
    #pragma omp target map(to: a[0:vN], b[0:vN]) map(from: result[0:vN])
    {
        #pragma omp parallel
        {
            #pragma omp simd private(i)
            for (int i = 0; i < vN; i++) {
                result[i] = a[i] + b[i] * 2.0f;
            }
        }
    }
}

/* Helper function to verify results */
int verify_results(int N, float *cpu_result, float *gpu_result, float tolerance) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabsf(cpu_result[i] - gpu_result[i]) > tolerance) {
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
        a2[i] = sinf((float)i * 0.01f);
        b2[i] = cosf((float)i * 0.02f);
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)i * 0.07f;
        b3[i] = (float)i * 0.03f;
    }
    
    /* Multiple invocations to hit different contexts */
    
    /* First: Primary SIMT transformation */
    printf("Running primary SIMT computation...\n");
    compute_simt(N1, a1, b1, result1);
    
    /* CPU verification for first kernel */
    cpu_compute(N1, a1, b1, cpu_result1);
    int errors1 = verify_results(N1, cpu_result1, result1, 1e-5f);
    
    /* Second: Nested teams/distribute structure */
    printf("Running nested SIMT computation...\n");
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Third: target simd with parallel region */
    printf("Running target SIMD computation...\n");
    compute_target_simd(N3, a3, b3, result3);
    
    /* Additional call from different control flow path */
    if (N1 > 100) {
        printf("Running conditional SIMT computation...\n");
        compute_simt(N1/2, a1, b1, result1);
    }
    
    /* Force compiler to consider alternative path (dead code that still gets parsed) */
    if (0) {  /* Will not execute but still compiled */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: a1[0:N1]) private(i)
        for (int i = 0; i < N1; i++) {
            a1[i] = 0.0f;
        }
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    printf("Test completed. Errors in first kernel: %d\n", errors1);
    return (errors1 > 0) ? 1 : 0;
}
