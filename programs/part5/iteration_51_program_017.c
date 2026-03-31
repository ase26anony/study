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
            private(i) shared(a,b,result) collapse(2)
        for (int i = 0; i < dyn_N; i++) {
            for (int j = 0; j < 8; j++) {
                result[i] = a[i] + b[i] * (float)j;
            }
        }
    }
}

/* Function with target simd that might trigger parallel SIMT */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int vN = N;
    
    #pragma omp target map(to: a[0:vN], b[0:vN]) map(from: result[0:vN])
    #pragma omp parallel for simd private(i) shared(result)
    for (int i = 0; i < vN; i++) {
        result[i] = a[i] - b[i];
    }
}

/* Helper function to verify results */
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
    /* Use non-constant sizes from command line or defaults */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 512;
    int N3 = (argc > 3) ? atoi(argv[3]) : 768;
    
    printf("Testing SIMT transformation with sizes: %d, %d, %d\n", N1, N2, N3);
    
    /* Allocate arrays with dynamic sizes */
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
        a1[i] = (float)i * 1.5f;
        b1[i] = (float)i * 0.5f;
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)i * 2.0f;
        b2[i] = (float)i * 0.25f;
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)i * 0.75f;
        b3[i] = (float)i * 1.25f;
    }
    
    /* Multiple invocations to hit different contexts */
    
    /* First call - primary SIMT pattern */
    printf("Call 1: target teams distribute parallel for simd\n");
    compute_simt(N1, a1, b1, result1);
    
    /* CPU reference for verification */
    cpu_compute(N1, a1, b1, cpu_result1);
    int errors = verify_results(cpu_result1, result1, N1);
    printf("Verification errors in call 1: %d\n", errors);
    
    /* Second call - nested pattern */
    printf("\nCall 2: nested teams/distribute parallel for simd\n");
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Third call - different size */
    printf("\nCall 3: target simd with parallel\n");
    compute_target_simd(N3, a3, b3, result3);
    
    /* Conditional block with alternative constructs */
    if (argc > 4) {
        /* This block exposes compiler to different syntax without execution */
        volatile int test_N = 256;
        float *test_a = (float *)malloc(test_N * sizeof(float));
        float *test_b = (float *)malloc(test_N * sizeof(float));
        float *test_result = (float *)malloc(test_N * sizeof(float));
        
        /* Alternative: target with teams and distribute separate */
        #pragma omp target teams map(to: test_a[0:test_N], test_b[0:test_N]) \
                                 map(from: test_result[0:test_N])
        {
            #pragma omp distribute simd
            for (int i = 0; i < test_N; i++) {
                test_result[i] = test_a[i] * test_b[i];
            }
        }
        
        free(test_a);
        free(test_b);
        free(test_result);
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    printf("\nTest completed\n");
    return 0;
}
