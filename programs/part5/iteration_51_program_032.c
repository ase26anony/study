/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT transformation target */
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
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    /* Explicitly nested teams and distribute parallel for simd */
    #pragma omp target map(to: a[0:size], b[0:size]) map(from: result[0:size])
    {
        #pragma omp teams num_teams(8)
        {
            #pragma omp distribute parallel for simd \
                private(i) shared(a, b, result) \
                collapse(2)
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < 10; j++) {
                    result[i] = a[i] * b[i] + j;
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int n = N;
    
    /* target simd in a context that suggests parallel execution */
    #pragma omp target teams distribute simd \
        map(to: a[0:n], b[0:n]) \
        map(from: result[0:n]) \
        private(i) \
        num_teams(2)
    for (int i = 0; i < n; i++) {
        result[i] = a[i] - b[i];
    }
}

/* Helper function to initialize arrays */
void init_arrays(float *a, float *b, int N) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
    }
}

/* Verification function */
int verify_results(float *cpu_result, float *gpu_result, int N) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (cpu_result[i] != gpu_result[i]) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                       i, cpu_result[i], gpu_result[i]);
            }
        }
    }
    return errors;
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent optimization */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 512;
    int N3 = (argc > 3) ? atoi(argv[3]) : 768;
    
    /* Dynamic allocation to prevent static analysis */
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
    
    if (!a1 || !b1 || !result1 || !cpu_result1 || 
        !a2 || !b2 || !result2 || 
        !a3 || !b3 || !result3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(a1, b1, N1);
    init_arrays(a2, b2, N2);
    init_arrays(a3, b3, N3);
    
    /* Multiple invocations to hit different contexts */
    
    /* First invocation - basic SIMT pattern */
    printf("Running compute_simt with N=%d\n", N1);
    compute_simt(N1, a1, b1, result1);
    
    /* Compute CPU reference */
    for (int i = 0; i < N1; i++) {
        cpu_result1[i] = a1[i] + b1[i];
    }
    
    /* Verify first computation */
    int errors = verify_results(cpu_result1, result1, N1);
    printf("Verification 1: %d errors\n", errors);
    
    /* Second invocation - nested pattern */
    printf("\nRunning compute_nested_simt with N=%d\n", N2);
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Third invocation - target simd pattern */
    printf("\nRunning compute_target_simd with N=%d\n", N3);
    compute_target_simd(N3, a3, b3, result3);
    
    /* Conditional block with alternative construct (kept for exposure) */
    if (0) {  /* Never executed but parsed by compiler */
        /* Alternative form that should also trigger SIMT transformation */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: a1[0:N1]) \
            collapse(2)
        for (int i = 0; i < N1/2; i++) {
            for (int j = 0; j < 2; j++) {
                a1[i*2 + j] *= 2.0f;
            }
        }
    }
    
    /* Additional test with function pointer to create different call context */
    void (*compute_func)(int, float*, float*, float*) = compute_simt;
    if (argc > 4) {
        compute_func = compute_nested_simt;
    }
    
    /* Call through function pointer */
    printf("\nRunning through function pointer\n");
    compute_func(N1/2, a1, b1, result1);
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    printf("\nTest completed\n");
    return 0;
}
