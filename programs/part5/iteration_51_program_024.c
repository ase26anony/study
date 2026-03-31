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
    volatile int dyn_N = N;
    
    #pragma omp target teams map(to: a[0:dyn_N], b[0:dyn_N]) map(from: result[0:dyn_N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(result) collapse(2)
        for (int i = 0; i < dyn_N; i++) {
            for (int j = 0; j < 8; j++) {
                result[i] = a[i] + b[i] * (float)j;
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int v_N = N;
    
    /* Create context that suggests parallel execution */
    #pragma omp target map(to: a[0:v_N], b[0:v_N]) map(from: result[0:v_N])
    {
        #pragma omp parallel
        {
            #pragma omp simd private(i)
            for (int i = 0; i < v_N; i++) {
                result[i] = a[i] - b[i];
            }
        }
    }
}

/* Helper for verification */
void verify_computation(int N, float *a, float *b, float *result, int mode) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = 0.0f;
        if (mode == 1) {
            for (int j = 0; j < 16; j++) {
                expected = a[i] * b[i] + (float)j * 0.1f;
            }
        } else if (mode == 2) {
            for (int j = 0; j < 8; j++) {
                expected = a[i] + b[i] * (float)j;
            }
        } else {
            expected = a[i] - b[i];
        }
        
        if (result[i] != expected) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at index %d: got %f, expected %f\n", 
                       i, result[i], expected);
            }
        }
    }
    if (errors > 0) {
        printf("Found %d errors in verification\n", errors);
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes from command line or defaults */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 512;
    int N3 = (argc > 3) ? atoi(argv[3]) : 256;
    
    printf("Testing SIMT transformations with sizes: %d, %d, %d\n", N1, N2, N3);
    
    /* Allocate and initialize arrays */
    float *a1 = (float *)malloc(N1 * sizeof(float));
    float *b1 = (float *)malloc(N1 * sizeof(float));
    float *r1 = (float *)malloc(N1 * sizeof(float));
    
    float *a2 = (float *)malloc(N2 * sizeof(float));
    float *b2 = (float *)malloc(N2 * sizeof(float));
    float *r2 = (float *)malloc(N2 * sizeof(float));
    
    float *a3 = (float *)malloc(N3 * sizeof(float));
    float *b3 = (float *)malloc(N3 * sizeof(float));
    float *r3 = (float *)malloc(N3 * sizeof(float));
    
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)i * 1.5f;
        b1[i] = (float)i * 0.5f;
    }
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)i * 2.0f;
        b2[i] = (float)i * 0.25f;
    }
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)i * 3.0f;
        b3[i] = (float)i * 0.75f;
    }
    
    /* Multiple invocations with different contexts */
    
    /* First: Direct SIMT construct */
    printf("Running compute_simt...\n");
    compute_simt(N1, a1, b1, r1);
    verify_computation(N1, a1, b1, r1, 1);
    
    /* Second: Nested teams/distribute */
    printf("Running compute_nested_simt...\n");
    compute_nested_simt(N2, a2, b2, r2);
    verify_computation(N2, a2, b2, r2, 2);
    
    /* Third: Alternative construct */
    printf("Running compute_target_simd...\n");
    compute_target_simd(N3, a3, b3, r3);
    verify_computation(N3, a3, b3, r3, 3);
    
    /* Additional call from different control flow path */
    if (N1 > 100) {
        printf("Running additional SIMT call from conditional path...\n");
        float *temp_a = (float *)malloc(128 * sizeof(float));
        float *temp_b = (float *)malloc(128 * sizeof(float));
        float *temp_r = (float *)malloc(128 * sizeof(float));
        
        for (int i = 0; i < 128; i++) {
            temp_a[i] = (float)i;
            temp_b[i] = (float)(i * 2);
        }
        
        compute_simt(128, temp_a, temp_b, temp_r);
        
        free(temp_a);
        free(temp_b);
        free(temp_r);
    }
    
    /* Cleanup */
    free(a1); free(b1); free(r1);
    free(a2); free(b2); free(r2);
    free(a3); free(b3); free(r3);
    
    printf("Test completed.\n");
    return 0;
}
