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

/* Alternative function with nested teams/distribute */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    #pragma omp target teams map(to: a[0:size], b[0:size]) map(from: result[0:size])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a, b, result) \
            num_threads(128)  /* Explicit thread count */
        for (int i = 0; i < size; i++) {
            result[i] = a[i] * 2.0f + b[i];
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int n = N;
    
    /* Create a parallel region that contains target simd */
    #pragma omp parallel shared(a, b, result, n)
    {
        #pragma omp target simd map(to: a[0:n], b[0:n]) map(from: result[0:n])
        for (int i = 0; i < n; i++) {
            result[i] = a[i] - b[i];
        }
    }
}

/* Helper function to verify results */
int verify_results(float *cpu_result, float *gpu_result, int N) {
    for (int i = 0; i < N; i++) {
        if (fabs(cpu_result[i] - gpu_result[i]) > 1e-5f) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes from command line or runtime */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 512;
    int N3 = (argc > 3) ? atoi(argv[3]) : 2048;
    
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
    
    /* Initialize arrays */
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)i * 1.5f;
        b1[i] = (float)i * 0.5f;
        cpu_result1[i] = a1[i] * b1[i];
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)(i % 100) * 0.1f;
        b2[i] = (float)(i % 50) * 0.2f;
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)i * 2.0f;
        b3[i] = (float)i * 3.0f;
    }
    
    /* Multiple invocations with different control flow */
    
    /* First: Direct call with combined construct */
    printf("Running combined SIMT construct...\n");
    compute_simt(N1, a1, b1, result1);
    
    /* Verify to prevent dead code elimination */
    if (verify_results(cpu_result1, result1, N1)) {
        printf("Test 1 passed\n");
    }
    
    /* Second: Conditional call path */
    if (N2 > 100) {
        printf("Running nested SIMT construct...\n");
        compute_nested_simt(N2, a2, b2, result2);
    }
    
    /* Third: Different function variant */
    printf("Running target SIMD construct...\n");
    compute_target_simd(N3, a3, b3, result3);
    
    /* Additional test: Inline complex construct */
    {
        volatile int local_N = 768;
        float local_a[768], local_b[768], local_result[768];
        
        for (int i = 0; i < local_N; i++) {
            local_a[i] = (float)i;
            local_b[i] = (float)(local_N - i);
        }
        
        /* Complex construct with multiple clauses */
        #pragma omp target teams distribute parallel for simd \
            map(to: local_a, local_b) map(from: local_result) \
            private(i) shared(local_result) \
            reduction(+:local_result[0:local_N]) \
            schedule(static, 32)
        for (int i = 0; i < local_N; i++) {
            local_result[i] = local_a[i] + local_b[i];
        }
        
        /* Use results to prevent optimization */
        float sum = 0.0f;
        for (int i = 0; i < local_N; i++) {
            sum += local_result[i];
        }
        printf("Inline test sum: %f\n", sum);
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    return 0;
}
