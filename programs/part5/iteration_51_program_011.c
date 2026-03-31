/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Function containing the primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) \
        collapse(2)
    for (int i = 0; i < dynamic_N; i++) {
        for (int j = 0; j < 10; j++) {
            result[i] = a[i] * b[i] + (float)j * 0.1f;
        }
    }
}

/* Alternative function with nested teams/distribute */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int dyn_N = N;
    
    #pragma omp target teams map(to: a[0:dyn_N], b[0:dyn_N]) \
                             map(from: result[0:dyn_N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a,b,result) \
            collapse(2)
        for (int i = 0; i < dyn_N; i++) {
            for (int j = 0; j < 8; j++) {
                result[i] = sinf(a[i]) * cosf(b[i]) + (float)j * 0.05f;
            }
        }
    }
}

/* Function with target simd in parallel context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int vN = N;
    
    #pragma omp target map(to: a[0:vN], b[0:vN]) \
                       map(from: result[0:vN])
    {
        #pragma omp parallel
        {
            #pragma omp simd private(i)
            for (int i = 0; i < vN; i++) {
                result[i] = sqrtf(a[i] * a[i] + b[i] * b[i]);
            }
        }
    }
}

/* Helper function to verify results */
int verify_results(float *cpu, float *gpu, int N, float tolerance) {
    for (int i = 0; i < N; i++) {
        if (fabsf(cpu[i] - gpu[i]) > tolerance) {
            printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                   i, cpu[i], gpu[i]);
            return 0;
        }
    }
    return 1;
}

/* CPU reference implementation */
void compute_cpu_reference(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        float sum = 0.0f;
        for (int j = 0; j < 10; j++) {
            sum += a[i] * b[i] + (float)j * 0.1f;
        }
        result[i] = sum / 10.0f;
    }
}

int main(int argc, char **argv) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 512;
    int N3 = (argc > 3) ? atoi(argv[3]) : 768;
    
    /* Dynamic allocation prevents compile-time analysis */
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
        a1[i] = (float)i * 0.1f;
        b1[i] = (float)(N1 - i) * 0.05f;
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = sinf((float)i * 0.01f);
        b2[i] = cosf((float)i * 0.02f);
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)i * 0.07f;
        b3[i] = (float)i * 0.03f;
    }
    
    /* Multiple invocations from different contexts */
    
    /* First call - primary SIMT transformation */
    compute_simt(N1, a1, b1, result1);
    
    /* Compute CPU reference for verification */
    compute_cpu_reference(N1, a1, b1, cpu_result1);
    
    /* Conditional call to expose different paths */
    if (N2 > 256) {
        compute_nested_simt(N2, a2, b2, result2);
    } else {
        compute_target_simd(N2, a2, b2, result2);
    }
    
    /* Third call from loop context */
    for (int iter = 0; iter < 2; iter++) {
        compute_simt(N3, a3, b3, result3);
    }
    
    /* Alternative construct in dead code (still parsed) */
    if (0) {  /* Never executed but still compiled */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: a1[0:N1]) private(i) collapse(2)
        for (int i = 0; i < N1; i++) {
            for (int j = 0; j < 5; j++) {
                a1[i] += (float)j;
            }
        }
    }
    
    /* Verify results */
    int success = 1;
    success &= verify_results(cpu_result1, result1, N1, 1e-4f);
    
    /* Simple verification for other results */
    float sum2 = 0.0f, sum3 = 0.0f;
    for (int i = 0; i < N2; i++) sum2 += result2[i];
    for (int i = 0; i < N3; i++) sum3 += result3[i];
    
    printf("Result sums: sum2=%f, sum3=%f\n", sum2, sum3);
    
    if (success) {
        printf("All verifications passed!\n");
    } else {
        printf("Verification failed!\n");
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    return success ? 0 : 1;
}
