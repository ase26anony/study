/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT */
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
        num_teams(256) thread_limit(128)
    for (int i = 0; i < dynamic_N; i++) {
        result[i] = a[i] * b[i] + (float)i / N;
    }
}

/* Alternative function with nested constructs */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    #pragma omp target teams map(to: a[0:size], b[0:size]) \
                             map(from: result[0:size]) \
                             num_teams(128)
    {
        #pragma omp distribute parallel for simd \
                private(i) shared(a,b,result) \
                collapse(2)
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < size) {
                    result[idx] = a[idx] + b[idx] * (float)j;
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int n = N;
    
    #pragma omp target map(to: a[0:n], b[0:n]) \
                       map(from: result[0:n])
    {
        #pragma omp parallel for simd private(i) shared(a,b,result)
        for (int i = 0; i < n; i++) {
            result[i] = a[i] - b[i];
        }
    }
}

/* Helper function to initialize arrays */
void init_arrays(int N, float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.5f;
    }
}

/* Verification function */
int verify_results(int N, float *result, float *expected) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float diff = result[i] - expected[i];
        if (diff > 0.001f || diff < -0.001f) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at %d: result=%f, expected=%f\n", 
                       i, result[i], expected[i]);
            }
        }
    }
    return errors;
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes from command line or defaults */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 2048;
    int N3 = (argc > 3) ? atoi(argv[3]) : 512;
    
    /* Allocate arrays with dynamic sizes */
    float *a1 = (float *)malloc(N1 * sizeof(float));
    float *b1 = (float *)malloc(N1 * sizeof(float));
    float *result1 = (float *)malloc(N1 * sizeof(float));
    float *expected1 = (float *)malloc(N1 * sizeof(float));
    
    float *a2 = (float *)malloc(N2 * sizeof(float));
    float *b2 = (float *)malloc(N2 * sizeof(float));
    float *result2 = (float *)malloc(N2 * sizeof(float));
    
    float *a3 = (float *)malloc(N3 * sizeof(float));
    float *b3 = (float *)malloc(N3 * sizeof(float));
    float *result3 = (float *)malloc(N3 * sizeof(float));
    
    if (!a1 || !b1 || !result1 || !expected1 || 
        !a2 || !b2 || !result2 ||
        !a3 || !b3 || !result3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(N1, a1, b1);
    init_arrays(N2, a2, b2);
    init_arrays(N3, a3, b3);
    
    /* Multiple invocations with different contexts */
    
    /* First: Primary SIMT transformation */
    printf("Running primary SIMT computation (N=%d)...\n", N1);
    compute_simt(N1, a1, b1, result1);
    
    /* Compute expected results on CPU for verification */
    for (int i = 0; i < N1; i++) {
        expected1[i] = a1[i] * b1[i] + (float)i / N1;
    }
    
    /* Second: Nested construct in conditional context */
    if (N2 > 0) {
        printf("Running nested SIMT computation (N=%d)...\n", N2);
        compute_nested_simt(N2, a2, b2, result2);
    }
    
    /* Third: Alternative construct from different call site */
    printf("Running target SIMD computation (N=%d)...\n", N3);
    compute_target_simd(N3, a3, b3, result3);
    
    /* Additional call to same function with different size */
    if (N1 > 100) {
        int smaller_N = N1 / 2;
        float *temp_result = (float *)malloc(smaller_N * sizeof(float));
        if (temp_result) {
            printf("Running additional SIMT computation (N=%d)...\n", smaller_N);
            compute_simt(smaller_N, a1, b1, temp_result);
            free(temp_result);
        }
    }
    
    /* Verify results */
    int errors = verify_results(N1, result1, expected1);
    if (errors == 0) {
        printf("Primary computation PASSED\n");
    } else {
        printf("Primary computation FAILED with %d errors\n", errors);
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(expected1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    return 0;
}
