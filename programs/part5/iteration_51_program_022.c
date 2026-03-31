/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT candidate construct */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) collapse(2)
    for (int i = 0; i < dynamic_N; i++) {
        for (int j = 0; j < 10; j++) {
            result[i] = a[i] * b[i] + (float)j;
        }
    }
}

/* Alternative function with nested teams/distribute */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int vol_N = N;
    
    #pragma omp target teams map(to: a[0:vol_N], b[0:vol_N]) map(from: result[0:vol_N])
    {
        #pragma omp distribute parallel for simd private(i) shared(result) collapse(2)
        for (int i = 0; i < vol_N; i++) {
            for (int j = 0; j < 8; j++) {
                result[i] = a[i] + b[i] * (float)j;
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    #pragma omp target map(to: a[0:size], b[0:size]) map(from: result[0:size])
    {
        #pragma omp parallel
        {
            #pragma omp for simd private(i)
            for (int i = 0; i < size; i++) {
                result[i] = a[i] - b[i];
            }
        }
    }
}

/* Helper function to initialize arrays */
void init_arrays(int N, float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)i * 2.0f;
    }
}

/* Verification function */
int verify_results(int N, float *result, float *expected) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (result[i] != expected[i]) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at index %d: got %f, expected %f\n", 
                       i, result[i], expected[i]);
            }
        }
    }
    return errors;
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 512;
    int N3 = (argc > 3) ? atoi(argv[3]) : 256;
    
    /* Dynamic allocation to avoid constant propagation */
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
    
    /* Initialize arrays with different patterns */
    init_arrays(N1, a1, b1);
    init_arrays(N2, a2, b2);
    init_arrays(N3, a3, b3);
    
    /* Test 1: Primary SIMT construct */
    printf("Test 1: Running compute_simt with N=%d\n", N1);
    compute_simt(N1, a1, b1, result1);
    
    /* Compute expected results for verification */
    for (int i = 0; i < N1; i++) {
        expected1[i] = a1[i] * b1[i] + 9.0f; /* Last iteration of inner loop */
    }
    
    int errors1 = verify_results(N1, result1, expected1);
    printf("Test 1 errors: %d\n", errors1);
    
    /* Test 2: Nested construct from different call site */
    printf("\nTest 2: Running compute_nested_simt with N=%d\n", N2);
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Test 3: Alternative construct in conditional context */
    printf("\nTest 3: Running compute_target_simd with N=%d\n", N3);
    if (N3 > 0) {  /* Ensure conditional execution */
        compute_target_simd(N3, a3, b3, result3);
    }
    
    /* Additional test with different control flow */
    printf("\nTest 4: Multiple invocations with varying sizes\n");
    for (int iter = 0; iter < 3; iter++) {
        int current_N = 128 * (iter + 1);
        float *tmp_a = (float *)malloc(current_N * sizeof(float));
        float *tmp_b = (float *)malloc(current_N * sizeof(float));
        float *tmp_res = (float *)malloc(current_N * sizeof(float));
        
        if (tmp_a && tmp_b && tmp_res) {
            init_arrays(current_N, tmp_a, tmp_b);
            
            /* Alternate between different constructs */
            if (iter % 2 == 0) {
                compute_simt(current_N, tmp_a, tmp_b, tmp_res);
            } else {
                compute_nested_simt(current_N, tmp_a, tmp_b, tmp_res);
            }
            
            free(tmp_a);
            free(tmp_b);
            free(tmp_res);
        }
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(expected1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    printf("\nAll tests completed\n");
    return 0;
}
