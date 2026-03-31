/* Test program to trigger SIMT transformation in GCC's OpenMP offloading */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Function containing the primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: result[0:N]) \
        private(i) shared(a, b, result) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < N) {
                result[idx] = a[idx] * b[idx] + sinf((float)idx);
            }
        }
    }
}

/* Alternative function with nested teams/distribute */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    #pragma omp target teams map(to: a[0:N], b[0:N]) map(from: result[0:N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a, b, result) collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < 8; j++) {
                int idx = i * 8 + j;
                if (idx < N) {
                    result[idx] = a[idx] / (b[idx] + 1.0f) + cosf((float)idx);
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    #pragma omp target map(to: a[0:N], b[0:N]) map(from: result[0:N])
    {
        #pragma omp parallel
        {
            #pragma omp simd private(i)
            for (int i = 0; i < N; i++) {
                result[i] = sqrtf(a[i] * a[i] + b[i] * b[i]);
            }
        }
    }
}

/* Helper function to initialize arrays */
void init_arrays(int N, float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 1) % 100) * 0.2f;
    }
}

/* Verification function */
int verify_results(int N, float *result, float *expected) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(result[i] - expected[i]) > 1e-6f) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at %d: %f != %f\n", i, result[i], expected[i]);
            }
        }
    }
    return errors;
}

/* Main function orchestrating multiple SIMT patterns */
int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent constant folding */
    int base_size = 1024;
    if (argc > 1) base_size = atoi(argv[1]);
    
    /* Make sizes volatile to prevent optimization */
    volatile int N1 = base_size;
    volatile int N2 = base_size * 2;
    volatile int N3 = base_size / 2;
    
    /* Allocate arrays dynamically */
    float *a1 = (float *)malloc(N1 * sizeof(float));
    float *b1 = (float *)malloc(N1 * sizeof(float));
    float *r1 = (float *)malloc(N1 * sizeof(float));
    float *expected1 = (float *)malloc(N1 * sizeof(float));
    
    float *a2 = (float *)malloc(N2 * sizeof(float));
    float *b2 = (float *)malloc(N2 * sizeof(float));
    float *r2 = (float *)malloc(N2 * sizeof(float));
    float *expected2 = (float *)malloc(N2 * sizeof(float));
    
    float *a3 = (float *)malloc(N3 * sizeof(float));
    float *b3 = (float *)malloc(N3 * sizeof(float));
    float *r3 = (float *)malloc(N3 * sizeof(float));
    float *expected3 = (float *)malloc(N3 * sizeof(float));
    
    if (!a1 || !b1 || !r1 || !expected1 || 
        !a2 || !b2 || !r2 || !expected2 ||
        !a3 || !b3 || !r3 || !expected3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(N1, a1, b1);
    init_arrays(N2, a2, b2);
    init_arrays(N3, a3, b3);
    
    /* Compute expected results on CPU for verification */
    for (int i = 0; i < N1; i++) {
        expected1[i] = a1[i] * b1[i] + sinf((float)i);
    }
    for (int i = 0; i < N2; i++) {
        expected2[i] = a2[i] / (b2[i] + 1.0f) + cosf((float)i);
    }
    for (int i = 0; i < N3; i++) {
        expected3[i] = sqrtf(a3[i] * a3[i] + b3[i] * b3[i]);
    }
    
    /* Multiple invocations to hit different contexts */
    printf("Running SIMT computation 1...\n");
    compute_simt(N1, a1, b1, r1);
    
    printf("Running SIMT computation 2...\n");
    compute_nested_simt(N2, a2, b2, r2);
    
    printf("Running SIMT computation 3...\n");
    compute_target_simd(N3, a3, b3, r3);
    
    /* Verify results */
    int errors1 = verify_results(N1, r1, expected1);
    int errors2 = verify_results(N2, r2, expected2);
    int errors3 = verify_results(N3, r3, expected3);
    
    printf("Verification results:\n");
    printf("  Pattern 1: %d errors\n", errors1);
    printf("  Pattern 2: %d errors\n", errors2);
    printf("  Pattern 3: %d errors\n", errors3);
    
    /* Conditional block with alternative constructs */
    if (argc > 2) {
        /* This block contains additional SIMT patterns that might be compiled
           but not executed, exposing the compiler to different syntax */
        #pragma omp target teams distribute parallel for simd \
            map(to: a1[0:N1]) map(from: r1[0:N1]) \
            private(i) collapse(2)
        for (int i = 0; i < N1; i++) {
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < N1) {
                    r1[idx] = a1[idx] * 2.0f;
                }
            }
        }
    }
    
    /* Cleanup */
    free(a1); free(b1); free(r1); free(expected1);
    free(a2); free(b2); free(r2); free(expected2);
    free(a3); free(b3); free(r3); free(expected3);
    
    return (errors1 + errors2 + errors3) > 0 ? 1 : 0;
}
