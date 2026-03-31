/* Test program to trigger SIMT transformation in GCC's OpenMP offloading */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT candidate construct */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding */
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

/* Alternative function with nested teams/distribute */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int vN = N;
    
    #pragma omp target teams map(tofrom: a[0:vN], b[0:vN], result[0:vN])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a,b,result) collapse(2)
        for (int i = 0; i < vN; i++) {
            for (int j = 0; j < 8; j++) {
                int idx = i * 8 + j;
                if (idx < vN) {
                    result[idx] = a[idx] + b[idx] * (float)(i + j);
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int loop_bound = N;
    
    /* Create context that suggests parallel execution */
    #pragma omp target map(tofrom: a[0:loop_bound], b[0:loop_bound], result[0:loop_bound])
    {
        #pragma omp parallel
        {
            #pragma omp simd private(i)
            for (int i = 0; i < loop_bound; i++) {
                result[i] = a[i] - b[i];
            }
        }
    }
}

/* Helper function to verify results */
int verify_results(int N, float *cpu_result, float *gpu_result) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float diff = cpu_result[i] - gpu_result[i];
        if (diff > 0.001f || diff < -0.001f) {
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
        cpu_result1[i] = 0.0f;
        result1[i] = 0.0f;
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)i * 2.0f;
        b2[i] = (float)i * 0.25f;
        result2[i] = 0.0f;
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)i * 1.2f;
        b3[i] = (float)i * 0.8f;
        result3[i] = 0.0f;
    }
    
    /* Multiple invocations to hit different contexts */
    printf("Running SIMT computation 1...\n");
    compute_simt(N1, a1, b1, result1);
    
    printf("Running nested SIMT computation...\n");
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Call from conditional branch */
    if (N3 > 256) {
        printf("Running target SIMD computation...\n");
        compute_target_simd(N3, a3, b3, result3);
    }
    
    /* Second call to same function with different data size */
    printf("Running SIMT computation again...\n");
    compute_simt(N2, a2, b2, result2);
    
    /* CPU verification */
    printf("Verifying results...\n");
    cpu_compute(N1, a1, b1, cpu_result1);
    int errors = verify_results(N1, cpu_result1, result1);
    
    if (errors == 0) {
        printf("All computations successful!\n");
    } else {
        printf("Found %d errors\n", errors);
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    return 0;
}
