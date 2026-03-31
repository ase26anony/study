/* Test program to trigger SIMT transformation in GCC's OpenMP lowering */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    /* This combined construct should trigger IFN_GOMP_USE_SIMT generation */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: a[0:N], b[0:N], result[0:N]) \
        private(i) shared(result) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < N) {
                result[idx] = a[idx] * b[idx] + (float)(i + j);
            }
        }
    }
}

/* Alternative function with nested explicit constructs */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    /* Explicit nesting: teams -> distribute -> parallel for simd */
    #pragma omp target teams map(tofrom: a[0:N], b[0:N], result[0:N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(result) collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < 8; j++) {
                int idx = i * 8 + j;
                if (idx < N) {
                    result[idx] = a[idx] / (b[idx] + 1.0f) + (float)j;
                }
            }
        }
    }
}

/* Function with target simd in parallel context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int use_parallel = 1; /* Prevent constant folding */
    
    if (use_parallel) {
        /* target simd with parallel execution context */
        #pragma omp target map(tofrom: a[0:N], b[0:N], result[0:N])
        #pragma omp parallel for simd private(i)
        for (int i = 0; i < N; i++) {
            result[i] = a[i] - b[i] * (float)i;
        }
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
void compute_cpu_reference(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < N) {
                result[idx] = a[idx] * b[idx] + (float)(i + j);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent loop elimination */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 512;
    int N3 = (argc > 3) ? atoi(argv[3]) : 768;
    
    /* Dynamic allocation prevents compile-time analysis */
    float *a1 = (float *)malloc(N1 * sizeof(float));
    float *b1 = (float *)malloc(N1 * sizeof(float));
    float *result1 = (float *)malloc(N1 * sizeof(float));
    float *cpu_ref1 = (float *)malloc(N1 * sizeof(float));
    
    float *a2 = (float *)malloc(N2 * sizeof(float));
    float *b2 = (float *)malloc(N2 * sizeof(float));
    float *result2 = (float *)malloc(N2 * sizeof(float));
    
    float *a3 = (float *)malloc(N3 * sizeof(float));
    float *b3 = (float *)malloc(N3 * sizeof(float));
    float *result3 = (float *)malloc(N3 * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)(i % 100);
        b1[i] = (float)((i + 1) % 100);
        cpu_ref1[i] = 0.0f;
        result1[i] = 0.0f;
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)(i % 50);
        b2[i] = (float)((i * 2) % 50);
        result2[i] = 0.0f;
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)(i % 75);
        b3[i] = (float)((i + 3) % 75);
        result3[i] = 0.0f;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Primary SIMT construct - should trigger the uncovered code */
    printf("Test 1: target teams distribute parallel for simd\n");
    compute_simt(N1, a1, b1, result1);
    
    /* CPU reference for verification */
    compute_cpu_reference(N1, a1, b1, cpu_ref1);
    int errors1 = verify_results(N1, cpu_ref1, result1, 0.001f);
    printf("Test 1 errors: %d\n", errors1);
    
    /* Test 2: Nested construct from different call site */
    printf("\nTest 2: Nested teams/distribute/parallel for simd\n");
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Test 3: Alternative construct in conditional context */
    printf("\nTest 3: target with parallel for simd\n");
    compute_target_simd(N3, a3, b3, result3);
    
    /* Additional invocation with different size to vary context */
    if (N1 > 100) {
        printf("\nAdditional test: Small size variant\n");
        float *small_a = (float *)malloc(100 * sizeof(float));
        float *small_b = (float *)malloc(100 * sizeof(float));
        float *small_result = (float *)malloc(100 * sizeof(float));
        
        for (int i = 0; i < 100; i++) {
            small_a[i] = (float)i;
            small_b[i] = (float)(100 - i);
        }
        
        compute_simt(100, small_a, small_b, small_result);
        
        free(small_a);
        free(small_b);
        free(small_result);
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_ref1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    printf("\nAll tests completed.\n");
    return 0;
}
