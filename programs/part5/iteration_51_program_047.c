/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary target region for SIMT transformation */
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
    volatile int dyn_N = N;
    
    #pragma omp target teams map(to: a[0:dyn_N], b[0:dyn_N]) \
                             map(from: result[0:dyn_N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a,b,result) collapse(2)
        for (int i = 0; i < dyn_N; i++) {
            for (int j = 0; j < 8; j++) {
                result[i] = a[i] + b[i] * (float)j;
            }
        }
    }
}

/* Function with target simd construct */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int vN = N;
    
    #pragma omp target map(to: a[0:vN], b[0:vN]) \
                       map(from: result[0:vN])
    #pragma omp parallel for simd private(i)
    for (int i = 0; i < vN; i++) {
        result[i] = a[i] * 2.0f + b[i];
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

/* CPU reference computation */
void cpu_compute(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            result[i] = a[i] * b[i] + (float)j * 0.1f;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command line argument for dynamic size */
    int N = 1024;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1024;
    }
    
    /* Dynamic allocation to prevent compile-time optimization */
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
        a[i] = (float)i * 0.5f;
        b[i] = (float)(N - i) * 0.3f;
        gpu_result1[i] = gpu_result2[i] = gpu_result3[i] = cpu_result[i] = 0.0f;
    }
    
    printf("Testing SIMT transformation with N=%d\n", N);
    
    /* Multiple invocations from different contexts */
    
    /* First call - primary SIMT construct */
    printf("1. Testing #pragma omp target teams distribute parallel for simd\n");
    compute_simt(N, a, b, gpu_result1);
    
    /* Second call - nested construct */
    printf("2. Testing nested teams/distribute parallel for simd\n");
    compute_nested_simt(N, a, b, gpu_result2);
    
    /* Third call - from conditional context */
    if (N > 512) {
        printf("3. Testing from conditional context\n");
        compute_target_simd(N, a, b, gpu_result3);
    }
    
    /* CPU reference computation */
    cpu_compute(N, a, b, cpu_result);
    
    /* Verification */
    int errors1 = verify_results(cpu_result, gpu_result1, N);
    int errors2 = verify_results(cpu_result, gpu_result2, N);
    int errors3 = verify_results(cpu_result, gpu_result3, N);
    
    printf("Verification results:\n");
    printf("  Method 1 errors: %d\n", errors1);
    printf("  Method 2 errors: %d\n", errors2);
    printf("  Method 3 errors: %d\n", errors3);
    
    /* Additional test with different data size */
    if (argc > 2) {
        int N2 = atoi(argv[2]);
        if (N2 > 0 && N2 != N) {
            printf("\nTesting with different size N=%d\n", N2);
            
            float *a2 = (float *)malloc(N2 * sizeof(float));
            float *b2 = (float *)malloc(N2 * sizeof(float));
            float *res2 = (float *)malloc(N2 * sizeof(float));
            
            if (a2 && b2 && res2) {
                for (int i = 0; i < N2; i++) {
                    a2[i] = (float)i * 0.7f;
                    b2[i] = (float)(N2 - i) * 0.4f;
                }
                
                compute_simt(N2, a2, b2, res2);
                
                free(a2);
                free(b2);
                free(res2);
            }
        }
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(gpu_result1);
    free(gpu_result2);
    free(gpu_result3);
    free(cpu_result);
    
    printf("Test completed\n");
    return 0;
}
