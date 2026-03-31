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
        private(i) shared(result) \
        num_teams(256) thread_limit(64)
    for (int i = 0; i < dynamic_N; i++) {
        result[i] = a[i] + b[i];
    }
}

/* Alternative function with nested constructs */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int n = N;
    
    #pragma omp target teams map(to: a[0:n], b[0:n]) map(from: result[0:n])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a, b, result) \
            collapse(2)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < 10; j++) {
                result[i] = a[i] * b[i] + j;
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
            #pragma omp simd
            for (int i = 0; i < size; i++) {
                result[i] = a[i] - b[i];
            }
        }
    }
}

/* Helper function to verify results */
int verify_results(int N, float *cpu_result, float *gpu_result) {
    for (int i = 0; i < N; i++) {
        if (cpu_result[i] != gpu_result[i]) {
            printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                   i, cpu_result[i], gpu_result[i]);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1000;
    int N2 = (argc > 2) ? atoi(argv[2]) : 2000;
    
    /* Dynamic allocation prevents constant propagation */
    float *a1 = (float *)malloc(N1 * sizeof(float));
    float *b1 = (float *)malloc(N1 * sizeof(float));
    float *result1 = (float *)malloc(N1 * sizeof(float));
    float *cpu_result1 = (float *)malloc(N1 * sizeof(float));
    
    float *a2 = (float *)malloc(N2 * sizeof(float));
    float *b2 = (float *)malloc(N2 * sizeof(float));
    float *result2 = (float *)malloc(N2 * sizeof(float));
    float *cpu_result2 = (float *)malloc(N2 * sizeof(float));
    
    /* Initialize arrays */
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)i;
        b1[i] = (float)(i * 2);
        cpu_result1[i] = a1[i] + b1[i];
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)(i % 100);
        b2[i] = (float)(i % 50);
        cpu_result2[i] = a2[i] * b2[i];
    }
    
    /* Multiple invocations to hit different contexts */
    printf("Running SIMT computation 1...\n");
    compute_simt(N1, a1, b1, result1);
    
    printf("Running nested SIMT computation...\n");
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Conditional execution path */
    if (N1 > 500) {
        printf("Running alternative SIMD construct...\n");
        compute_target_simd(N1, a1, b1, result1);
    }
    
    /* Verify results - prevents dead code elimination */
    int success1 = verify_results(N1, cpu_result1, result1);
    int success2 = verify_results(N2, cpu_result2, result2);
    
    if (success1 && success2) {
        printf("All computations verified successfully!\n");
    } else {
        printf("Verification failed!\n");
    }
    
    /* Additional test with different data sizes */
    printf("Running additional SIMT tests...\n");
    for (int test = 0; test < 3; test++) {
        int test_size = 100 * (test + 1);
        float *test_a = (float *)malloc(test_size * sizeof(float));
        float *test_b = (float *)malloc(test_size * sizeof(float));
        float *test_result = (float *)malloc(test_size * sizeof(float));
        
        for (int i = 0; i < test_size; i++) {
            test_a[i] = (float)(test * i);
            test_b[i] = (float)(test * i + 1);
        }
        
        compute_simt(test_size, test_a, test_b, test_result);
        
        free(test_a);
        free(test_b);
        free(test_result);
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2); free(cpu_result2);
    
    return 0;
}
