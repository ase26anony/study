/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    /* Primary target: teams distribute parallel for simd with explicit clauses */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) collapse(2) num_teams(128) thread_limit(64)
    for (int i = 0; i < dynamic_N; i++) {
        for (int j = 0; j < 10; j++) {
            result[i] = a[i] * b[i] + (float)j * 0.1f;
        }
    }
}

/* Alternative function with nested teams/distribute constructs */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int dyn_N = N;
    
    /* Nested approach: teams region containing distribute parallel for simd */
    #pragma omp target teams map(tofrom: a[0:dyn_N], b[0:dyn_N]) \
                             map(from: result[0:dyn_N]) num_teams(64)
    {
        #pragma omp distribute parallel for simd \
                private(i) shared(result) collapse(2)
        for (int i = 0; i < dyn_N; i++) {
            for (int j = 0; j < 8; j++) {
                result[i] = a[i] + b[i] * (float)(i + j);
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int vN = N;
    
    /* target simd in a context that suggests parallel GPU execution */
    #pragma omp target map(tofrom: a[0:vN], b[0:vN]) map(from: result[0:vN])
    {
        #pragma omp parallel for simd private(i) shared(result)
        for (int i = 0; i < vN; i++) {
            result[i] = (a[i] * a[i]) + (b[i] * b[i]);
        }
    }
}

/* Helper function to verify results */
int verify_results(int N, float *cpu_result, float *gpu_result, float tolerance) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(cpu_result[i] - gpu_result[i]) > tolerance) {
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
        for (int j = 0; j < 10; j++) {
            result[i] = a[i] * b[i] + (float)j * 0.1f;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 2048;
    int N3 = (argc > 3) ? atoi(argv[3]) : 512;
    
    /* Dynamic allocation prevents constant propagation */
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
        a1[i] = (float)i * 0.5f;
        b1[i] = (float)i * 1.5f;
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)(i % 100) * 0.25f;
        b2[i] = (float)(i % 50) * 2.0f;
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)i * 0.75f;
        b3[i] = (float)i * 0.33f;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Primary SIMT transformation pattern */
    printf("Test 1: target teams distribute parallel for simd\n");
    compute_simt(N1, a1, b1, result1);
    
    /* CPU verification for Test 1 */
    cpu_compute(N1, a1, b1, cpu_result1);
    int errors1 = verify_results(N1, cpu_result1, result1, 1e-5f);
    printf("Test 1 errors: %d\n", errors1);
    
    /* Test 2: Nested teams/distribute pattern from different call site */
    printf("\nTest 2: nested teams/distribute parallel for simd\n");
    if (N2 > 0) {  /* Conditional execution to vary context */
        compute_nested_simt(N2, a2, b2, result2);
    }
    
    /* Test 3: target simd with parallel context */
    printf("\nTest 3: target simd with parallel execution\n");
    compute_target_simd(N3, a3, b3, result3);
    
    /* Additional invocation with different parameters */
    printf("\nAdditional invocation with swapped sizes...\n");
    compute_simt(N3, a3, b3, result3);
    compute_nested_simt(N1, a1, b1, result1);
    
    /* Dead code that won't execute but exposes syntax to compiler */
    if (0) {
        /* Expose alternative syntax without executing */
        #pragma omp target teams distribute simd parallel for \
                map(to: a1[0:N1]) map(from: result1[0:N1])
        for (int i = 0; i < N1; i++) {
            result1[i] = a1[i] * 2.0f;
        }
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    printf("\nSIMT transformation tests completed.\n");
    return 0;
}
