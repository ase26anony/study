/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT target region */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    /* Primary target region with teams distribute parallel for simd */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) \
        num_teams(128) thread_limit(64)
    for (int i = 0; i < dynamic_N; i++) {
        result[i] = a[i] + b[i] * 2.0f;
    }
}

/* Alternative function with nested constructs */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int dyn_N = N;
    
    /* Explicitly nested teams and distribute parallel for simd */
    #pragma omp target teams map(to: a[0:dyn_N], b[0:dyn_N]) \
                             map(from: result[0:dyn_N]) \
                             num_teams(256)
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(result) \
            collapse(2)
        for (int i = 0; i < dyn_N; i++) {
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < dyn_N) {
                    result[idx] = a[idx] * b[idx] - (float)j;
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int v_N = N;
    
    /* target simd in a context that suggests parallel execution */
    #pragma omp target map(to: a[0:v_N], b[0:v_N]) \
                       map(from: result[0:v_N])
    #pragma omp parallel for simd private(i) shared(result)
    for (int i = 0; i < v_N; i++) {
        result[i] = a[i] / (b[i] + 1.0f);
    }
}

/* Helper function to verify results */
int verify_results(int N, float *cpu_result, float *gpu_result, float tolerance) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float diff = cpu_result[i] - gpu_result[i];
        if (diff < -tolerance || diff > tolerance) {
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
        result[i] = a[i] + b[i] * 2.0f;
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent optimization */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 2048;
    int N3 = (argc > 3) ? atoi(argv[3]) : 512;
    
    /* Dynamic allocation to avoid constant propagation */
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
        a1[i] = (float)i;
        b1[i] = (float)(N1 - i);
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)(i * 2);
        b2[i] = (float)(i % 100);
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)(i * 3);
        b3[i] = (float)(i % 50);
    }
    
    /* Multiple invocations to hit different contexts */
    
    /* First invocation - basic SIMT pattern */
    printf("Running basic SIMT computation...\n");
    compute_simt(N1, a1, b1, result1);
    
    /* CPU verification */
    cpu_compute(N1, a1, b1, cpu_result1);
    int errors1 = verify_results(N1, cpu_result1, result1, 0.001f);
    printf("Basic SIMT: %d errors\n", errors1);
    
    /* Second invocation - nested pattern */
    printf("Running nested SIMT computation...\n");
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Third invocation - target simd pattern */
    printf("Running target SIMD computation...\n");
    compute_target_simd(N3, a3, b3, result3);
    
    /* Conditional execution to create different control flow contexts */
    if (N1 > 1000) {
        printf("Running additional SIMT in conditional context...\n");
        compute_simt(N1/2, a1, b1, result1);
    }
    
    /* Loop with multiple invocations */
    for (int iter = 0; iter < 3; iter++) {
        printf("Iteration %d: ", iter);
        compute_simt(N1/(iter+1), a1, b1, result1);
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    printf("Test completed.\n");
    return 0;
}
