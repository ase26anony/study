/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary target region for SIMT transformation */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    /* Primary construct: target teams distribute parallel for simd
       This is the main candidate for SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) \
        num_teams(8) thread_limit(128)
    for (int i = 0; i < dynamic_N; i++) {
        result[i] = a[i] * b[i] + (float)i;
    }
}

/* Alternative function with nested constructs */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int dyn_N = N;
    
    /* Nested teams and distribute parallel for simd */
    #pragma omp target teams map(to: a[0:dyn_N], b[0:dyn_N]) \
                             map(from: result[0:dyn_N]) \
                             num_teams(4)
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(result) \
            collapse(2)
        for (int i = 0; i < dyn_N/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < dyn_N) {
                    result[idx] = a[idx] * 2.0f + b[idx] * 3.0f;
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int vN = N;
    
    /* target simd in a context that suggests parallel execution */
    #pragma omp target map(to: a[0:vN], b[0:vN]) map(from: result[0:vN])
    #pragma omp parallel for simd private(i)
    for (int i = 0; i < vN; i++) {
        result[i] = (a[i] + b[i]) * (float)(i % 16);
    }
}

/* Helper function to verify results */
int verify_results(int N, float *cpu_result, float *gpu_result) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float diff = cpu_result[i] - gpu_result[i];
        if (diff < -0.001f || diff > 0.001f) {
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
        result[i] = a[i] * b[i] + (float)i;
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent loop elimination */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 2048;
    int N3 = (argc > 3) ? atoi(argv[3]) : 512;
    
    /* Dynamic allocation to avoid constant propagation */
    float *a1 = (float*)malloc(N1 * sizeof(float));
    float *b1 = (float*)malloc(N1 * sizeof(float));
    float *result1 = (float*)malloc(N1 * sizeof(float));
    float *cpu_result1 = (float*)malloc(N1 * sizeof(float));
    
    float *a2 = (float*)malloc(N2 * sizeof(float));
    float *b2 = (float*)malloc(N2 * sizeof(float));
    float *result2 = (float*)malloc(N2 * sizeof(float));
    
    float *a3 = (float*)malloc(N3 * sizeof(float));
    float *b3 = (float*)malloc(N3 * sizeof(float));
    float *result3 = (float*)malloc(N3 * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)(i % 100);
        b1[i] = (float)((i + 1) % 100);
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)(i % 50);
        b2[i] = (float)((i * 2) % 50);
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)(i % 25);
        b3[i] = (float)((i + 3) % 25);
    }
    
    /* Multiple invocations from different contexts */
    
    /* First invocation - primary SIMT candidate */
    printf("Running primary SIMT computation (N=%d)...\n", N1);
    compute_simt(N1, a1, b1, result1);
    
    /* CPU reference for verification */
    cpu_compute(N1, a1, b1, cpu_result1);
    
    /* Second invocation - nested construct */
    printf("Running nested SIMT computation (N=%d)...\n", N2);
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Third invocation - target simd variant */
    printf("Running target SIMD computation (N=%d)...\n", N3);
    compute_target_simd(N3, a3, b3, result3);
    
    /* Conditional execution to create different control flow contexts */
    if (N1 > 100) {
        printf("Running additional SIMT computation with conditional...\n");
        compute_simt(N1/2, a1, b1, result1);
    }
    
    /* Verification */
    int errors = verify_results(N1, cpu_result1, result1);
    if (errors == 0) {
        printf("Primary computation: SUCCESS\n");
    } else {
        printf("Primary computation: %d errors\n", errors);
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    /* Additional test case in dead code to expose compiler to syntax */
    if (0) {  /* Never executed, but parsed by compiler */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: a1[0:100]) collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                int idx = i * 10 + j;
                a1[idx] = 0.0f;
            }
        }
    }
    
    return 0;
}
