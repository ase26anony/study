/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary target region for SIMT transformation */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int n_vol = N;
    int n = n_vol;
    
    /* Primary construct: target teams distribute parallel for simd 
       This should trigger the SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: result[0:n]) \
        private(i) shared(result) num_teams(128) thread_limit(64)
    for (int i = 0; i < n; i++) {
        result[i] = a[i] + b[i];
    }
}

/* Alternative function with nested constructs */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int n_vol = N;
    int n = n_vol;
    
    /* Nested teams and distribute parallel for simd */
    #pragma omp target teams map(to: a[0:n], b[0:n]) map(from: result[0:n]) \
        num_teams(64)
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a,b,result) 
        for (int i = 0; i < n; i++) {
            result[i] = a[i] * b[i];
        }
    }
}

/* Function with collapse clause for more complex transformation */
void compute_collapsed_simt(int M, int N, float *a, float *b, float *result) {
    volatile int m_vol = M;
    volatile int n_vol = N;
    int m = m_vol;
    int n = n_vol;
    
    /* Collapsed loops for 2D computation */
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: a[0:m*n], b[0:m*n]) map(from: result[0:m*n]) \
        private(i,j) shared(result) 
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            int idx = i * n + j;
            result[idx] = a[idx] - b[idx];
        }
    }
}

/* Function with conditional execution path */
void compute_conditional_simt(int N, float *a, float *b, float *result, int mode) {
    volatile int n_vol = N;
    int n = n_vol;
    
    if (mode > 0) {
        /* This path should still trigger SIMT transformation */
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: result[0:n]) \
            private(i) shared(result)
        for (int i = 0; i < n; i++) {
            result[i] = a[i] / (b[i] + 1.0f);
        }
    } else {
        /* Alternative: target simd with parallel execution context */
        #pragma omp target map(to: a[0:n], b[0:n]) map(from: result[0:n])
        #pragma omp parallel for simd private(i) shared(a,b,result)
        for (int i = 0; i < n; i++) {
            result[i] = a[i] * b[i] * 2.0f;
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
        result[i] = a[i] + b[i];
    }
}

int main() {
    /* Use non-constant sizes to prevent loop elimination */
    int N1 = 1000;
    int N2 = 512;
    int M = 32;
    
    /* Dynamic allocation to avoid constant propagation */
    float *a1 = (float*)malloc(N1 * sizeof(float));
    float *b1 = (float*)malloc(N1 * sizeof(float));
    float *result1_gpu = (float*)malloc(N1 * sizeof(float));
    float *result1_cpu = (float*)malloc(N1 * sizeof(float));
    
    float *a2 = (float*)malloc(N2 * sizeof(float));
    float *b2 = (float*)malloc(N2 * sizeof(float));
    float *result2 = (float*)malloc(N2 * sizeof(float));
    
    float *a3 = (float*)malloc(M * N2 * sizeof(float));
    float *b3 = (float*)malloc(M * N2 * sizeof(float));
    float *result3 = (float*)malloc(M * N2 * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)i;
        b1[i] = (float)(i * 2);
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)(i + 1);
        b2[i] = (float)(i * 3);
    }
    
    for (int i = 0; i < M * N2; i++) {
        a3[i] = (float)i;
        b3[i] = (float)(i * 0.5f);
    }
    
    /* Multiple invocations to hit different contexts */
    
    /* First invocation - basic SIMT pattern */
    printf("Test 1: Basic target teams distribute parallel for simd\n");
    compute_simt(N1, a1, b1, result1_gpu);
    
    /* CPU reference */
    cpu_compute(N1, a1, b1, result1_cpu);
    
    /* Verification */
    int errors = verify_results(N1, result1_cpu, result1_gpu, 0.001f);
    printf("Test 1 errors: %d\n", errors);
    
    /* Second invocation - nested pattern */
    printf("\nTest 2: Nested teams + distribute parallel for simd\n");
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Third invocation - collapsed loops */
    printf("\nTest 3: Collapsed(2) target teams distribute parallel for simd\n");
    compute_collapsed_simt(M, N2, a3, b3, result3);
    
    /* Fourth invocation - conditional path */
    printf("\nTest 4: Conditional SIMT execution\n");
    compute_conditional_simt(N1, a1, b1, result1_gpu, 1);
    compute_conditional_simt(N1, a1, b1, result1_gpu, 0);
    
    /* Cleanup */
    free(a1); free(b1); free(result1_gpu); free(result1_cpu);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    printf("\nAll tests completed\n");
    return 0;
}
