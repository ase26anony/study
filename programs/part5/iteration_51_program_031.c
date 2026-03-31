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

/* Alternative function with nested teams/distribute construct */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    #pragma omp target teams map(to: a[0:size], b[0:size]) map(from: result[0:size])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a, b, result) collapse(2)
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < 8; j++) {
                result[i] = a[i] + b[i] * (float)j;
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int n = N;
    
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: result[0:n])
    {
        #pragma omp parallel
        {
            #pragma omp for simd private(i) shared(a, b, result)
            for (int i = 0; i < n; i++) {
                result[i] = a[i] - b[i];
            }
        }
    }
}

/* Verification function */
int verify_results(float *cpu_result, float *gpu_result, int N) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(cpu_result[i] - gpu_result[i]) > 1e-6) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at %d: CPU=%f, GPU=%f\n", 
                       i, cpu_result[i], gpu_result[i]);
            }
        }
    }
    return errors;
}

/* CPU reference computation */
void cpu_compute(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            result[i] = a[i] * b[i] + (float)j * 0.1f;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant size from command line or default */
    int N = 1024;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1024;
    }
    
    /* Dynamic allocation to prevent compile-time optimization */
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *result_gpu = (float *)malloc(N * sizeof(float));
    float *result_cpu = (float *)malloc(N * sizeof(float));
    
    if (!a || !b || !result_gpu || !result_cpu) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.7f;
        result_gpu[i] = 0.0f;
        result_cpu[i] = 0.0f;
    }
    
    printf("Testing SIMT transformation with N = %d\n", N);
    
    /* Multiple invocations to hit different contexts */
    
    /* First call - primary SIMT construct */
    compute_simt(N, a, b, result_gpu);
    
    /* CPU verification */
    cpu_compute(N, a, b, result_cpu);
    int errors = verify_results(result_cpu, result_gpu, N);
    printf("Test 1 errors: %d\n", errors);
    
    /* Reset results */
    for (int i = 0; i < N; i++) {
        result_gpu[i] = 0.0f;
        result_cpu[i] = 0.0f;
    }
    
    /* Second call - nested construct */
    compute_nested_simt(N, a, b, result_gpu);
    
    /* Different CPU computation for nested version */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 8; j++) {
            result_cpu[i] = a[i] + b[i] * (float)j;
        }
    }
    
    errors = verify_results(result_cpu, result_gpu, N);
    printf("Test 2 errors: %d\n", errors);
    
    /* Third call - conditional execution path */
    volatile int use_alternative = 1;
    if (use_alternative) {
        compute_target_simd(N, a, b, result_gpu);
        
        /* CPU computation for target simd version */
        for (int i = 0; i < N; i++) {
            result_cpu[i] = a[i] - b[i];
        }
        
        errors = verify_results(result_cpu, result_gpu, N);
        printf("Test 3 errors: %d\n", errors);
    }
    
    /* Additional test with different size */
    if (N > 100) {
        int half_N = N / 2;
        compute_simt(half_N, a, b, result_gpu);
        printf("Additional test with size %d completed\n", half_N);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(result_gpu);
    free(result_cpu);
    
    printf("All tests completed\n");
    return 0;
}
