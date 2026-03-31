/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary target region for SIMT transformation */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    /* Primary target region with teams distribute parallel for simd */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < dynamic_N; i++) {
        result[i] = a[i] + b[i];
    }
}

/* Alternative function with nested constructs */
void compute_nested(int N, float *a, float *b, float *result) {
    volatile int dynamic_N = N;
    
    /* Explicitly nested teams and distribute parallel for simd */
    #pragma omp target teams map(to: a[0:dynamic_N], b[0:dynamic_N]) \
                             map(from: result[0:dynamic_N]) \
                             num_teams(4)
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(result) \
            thread_limit(128)
        for (int i = 0; i < dynamic_N; i++) {
            result[i] = a[i] * b[i];
        }
    }
}

/* Function with 2D collapsed loop for more complex transformation */
void compute_collapsed(int M, int N, float *a, float *b, float *result) {
    volatile int dynamic_M = M;
    volatile int dynamic_N = N;
    int total_size = dynamic_M * dynamic_N;
    
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: a[0:total_size], b[0:total_size]) \
        map(from: result[0:total_size]) \
        private(i,j) shared(result) \
        num_teams(8) thread_limit(256)
    for (int i = 0; i < dynamic_M; i++) {
        for (int j = 0; j < dynamic_N; j++) {
            int idx = i * dynamic_N + j;
            result[idx] = a[idx] - b[idx];
        }
    }
}

/* Function with conditional SIMD region */
void compute_conditional(int N, float *a, float *b, float *result, int use_simd) {
    volatile int dynamic_N = N;
    
    if (use_simd) {
        /* This should trigger the conditional SIMT transformation */
        #pragma omp target simd \
            map(to: a[0:dynamic_N], b[0:dynamic_N]) \
            map(from: result[0:dynamic_N]) \
            private(i) shared(result)
        for (int i = 0; i < dynamic_N; i++) {
            result[i] = a[i] / (b[i] + 1.0f);
        }
    } else {
        /* Regular parallel version */
        #pragma omp target teams distribute parallel for \
            map(to: a[0:dynamic_N], b[0:dynamic_N]) \
            map(from: result[0:dynamic_N]) \
            private(i) shared(result)
        for (int i = 0; i < dynamic_N; i++) {
            result[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* Verification function */
int verify_results(int N, float *cpu_result, float *gpu_result, float tolerance) {
    for (int i = 0; i < N; i++) {
        if (fabs(cpu_result[i] - gpu_result[i]) > tolerance) {
            printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                   i, cpu_result[i], gpu_result[i]);
            return 0;
        }
    }
    return 1;
}

int main() {
    /* Use non-constant sizes to prevent optimization */
    int N1 = 1024;
    int N2 = 2048;
    int M = 64;
    int N = 32;
    
    /* Allocate and initialize arrays */
    float *a1 = (float *)malloc(N1 * sizeof(float));
    float *b1 = (float *)malloc(N1 * sizeof(float));
    float *result1 = (float *)malloc(N1 * sizeof(float));
    float *cpu_result1 = (float *)malloc(N1 * sizeof(float));
    
    float *a2 = (float *)malloc(N2 * sizeof(float));
    float *b2 = (float *)malloc(N2 * sizeof(float));
    float *result2 = (float *)malloc(N2 * sizeof(float));
    
    int total_size = M * N;
    float *a3 = (float *)malloc(total_size * sizeof(float));
    float *b3 = (float *)malloc(total_size * sizeof(float));
    float *result3 = (float *)malloc(total_size * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)i;
        b1[i] = (float)(i * 2);
        cpu_result1[i] = a1[i] + b1[i];  /* CPU reference */
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)(i % 100);
        b2[i] = (float)(i % 50);
    }
    
    for (int i = 0; i < total_size; i++) {
        a3[i] = (float)i;
        b3[i] = (float)(total_size - i);
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT transformation */
    printf("Test 1: Basic teams distribute parallel for simd\n");
    compute_simt(N1, a1, b1, result1);
    
    /* Verify results */
    if (verify_results(N1, cpu_result1, result1, 0.001f)) {
        printf("Test 1 passed\n");
    } else {
        printf("Test 1 failed\n");
    }
    
    /* Test 2: Nested construct */
    printf("\nTest 2: Nested teams and distribute parallel for simd\n");
    compute_nested(N2, a2, b2, result2);
    
    /* Test 3: Collapsed loop */
    printf("\nTest 3: Collapsed(2) teams distribute parallel for simd\n");
    compute_collapsed(M, N, a3, b3, result3);
    
    /* Test 4: Conditional SIMD region */
    printf("\nTest 4: Conditional SIMD region\n");
    compute_conditional(N1, a1, b1, result1, 1);
    compute_conditional(N1, a1, b1, result1, 0);
    
    /* Additional test with different control flow */
    printf("\nTest 5: Multiple calls from different contexts\n");
    for (int iter = 0; iter < 3; iter++) {
        if (iter % 2 == 0) {
            compute_simt(N1, a1, b1, result1);
        } else {
            compute_nested(N1, a1, b1, result1);
        }
    }
    
    /* Cleanup */
    free(a1);
    free(b1);
    free(result1);
    free(cpu_result1);
    free(a2);
    free(b2);
    free(result2);
    free(a3);
    free(b3);
    free(result3);
    
    printf("\nAll tests completed\n");
    return 0;
}
