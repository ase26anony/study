/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT generation */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    /* Primary target region designed to trigger SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) \
        num_teams(256) thread_limit(64)
    for (int i = 0; i < dynamic_N; i++) {
        result[i] = a[i] + b[i];
    }
}

/* Alternative function with nested constructs */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int rows = N / 16;
    volatile int cols = 16;
    
    /* Nested teams and distribute parallel for simd */
    #pragma omp target teams map(to: a[0:N], b[0:N]) map(from: result[0:N]) \
        num_teams(128)
    {
        #pragma omp distribute parallel for simd collapse(2) \
            private(i, j) shared(a, b, result)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                if (idx < N) {
                    result[idx] = a[idx] * b[idx];
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int block_size = 32;
    
    /* target simd in a context that suggests parallel execution */
    #pragma omp target map(to: a[0:N], b[0:N]) map(from: result[0:N])
    {
        #pragma omp parallel for simd simdlen(8)
        for (int i = 0; i < N; i += block_size) {
            for (int j = 0; j < block_size && (i + j) < N; j++) {
                result[i + j] = a[i + j] - b[i + j];
            }
        }
    }
}

/* Verification function */
int verify_results(int N, float *cpu_result, float *gpu_result, float tolerance) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float diff = cpu_result[i] - gpu_result[i];
        if (diff < -tolerance || diff > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                       i, cpu_result[i], gpu_result[i]);
            }
        }
    }
    return errors;
}

/* CPU reference computation */
void cpu_compute(int N, float *a, float *b, float *result, int mode) {
    switch (mode) {
        case 0: /* Addition */
            for (int i = 0; i < N; i++) {
                result[i] = a[i] + b[i];
            }
            break;
        case 1: /* Multiplication */
            for (int i = 0; i < N; i++) {
                result[i] = a[i] * b[i];
            }
            break;
        case 2: /* Subtraction */
            for (int i = 0; i < N; i++) {
                result[i] = a[i] - b[i];
            }
            break;
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant size to prevent loop elimination */
    int N = 1024;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1024;
    }
    
    /* Dynamic allocation to prevent static analysis */
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *gpu_result1 = (float *)malloc(N * sizeof(float));
    float *gpu_result2 = (float *)malloc(N * sizeof(float));
    float *gpu_result3 = (float *)malloc(N * sizeof(float));
    float *cpu_result = (float *)malloc(N * sizeof(float));
    
    if (!a || !b || !gpu_result1 || !gpu_result2 || !gpu_result3 || !cpu_result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100) * 1.5f;
        b[i] = (float)((i + 1) % 100) * 0.7f;
        gpu_result1[i] = gpu_result2[i] = gpu_result3[i] = cpu_result[i] = 0.0f;
    }
    
    printf("Testing SIMT transformation with N = %d\n", N);
    
    /* Multiple invocations to hit different contexts */
    
    /* First invocation - basic SIMT pattern */
    printf("1. Testing #pragma omp target teams distribute parallel for simd\n");
    compute_simt(N, a, b, gpu_result1);
    cpu_compute(N, a, b, cpu_result, 0);
    int errors1 = verify_results(N, cpu_result, gpu_result1, 0.001f);
    
    /* Second invocation - nested pattern */
    printf("2. Testing nested teams/distribute parallel for simd\n");
    compute_nested_simt(N, a, b, gpu_result2);
    cpu_compute(N, a, b, cpu_result, 1);
    int errors2 = verify_results(N, cpu_result, gpu_result2, 0.001f);
    
    /* Third invocation - target simd with parallel */
    printf("3. Testing target simd with parallel execution\n");
    compute_target_simd(N, a, b, gpu_result3);
    cpu_compute(N, a, b, cpu_result, 2);
    int errors3 = verify_results(N, cpu_result, gpu_result3, 0.001f);
    
    /* Conditional block with alternative constructs */
    if (argc > 2) {
        /* This block exposes the compiler to additional patterns */
        volatile int small_N = 64;
        float small_a[64], small_b[64], small_result[64];
        
        #pragma omp target teams distribute parallel for simd \
            map(to: small_a[0:64], small_b[0:64]) \
            map(from: small_result[0:64])
        for (int i = 0; i < small_N; i++) {
            small_result[i] = small_a[i] + small_b[i];
        }
    }
    
    /* Summary */
    printf("\nVerification Results:\n");
    printf("  Pattern 1 errors: %d\n", errors1);
    printf("  Pattern 2 errors: %d\n", errors2);
    printf("  Pattern 3 errors: %d\n", errors3);
    
    int total_errors = errors1 + errors2 + errors3;
    if (total_errors == 0) {
        printf("\nAll SIMT transformations completed successfully!\n");
    } else {
        printf("\nTotal errors: %d\n", total_errors);
    }
    
    /* Cleanup */
    free(a); free(b); 
    free(gpu_result1); free(gpu_result2); free(gpu_result3);
    free(cpu_result);
    
    return total_errors > 0 ? 1 : 0;
}
