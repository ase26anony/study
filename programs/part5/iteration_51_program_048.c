/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT generation */
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
        result[i] = a[i] + b[i] * 2.0f;
    }
}

/* Alternative function with nested constructs */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int dyn_N = N;
    
    /* Explicit nesting: target teams containing distribute parallel for simd */
    #pragma omp target teams map(to: a[0:dyn_N], b[0:dyn_N]) \
                             map(from: result[0:dyn_N]) \
                             num_teams(8)
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a,b,result) \
            collapse(2)
        for (int i = 0; i < dyn_N; i++) {
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < dyn_N) {
                    result[idx] = a[idx] - b[idx] / (j + 1);
                }
            }
        }
    }
}

/* Function with target simd in parallel context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int vN = N;
    
    /* target simd with parallel execution characteristics */
    #pragma omp target map(to: a[0:vN], b[0:vN]) \
                       map(from: result[0:vN]) \
                       device(0)
    #pragma omp parallel for simd private(i) \
        if(vN > 1000)  /* Conditional to create branching */
    for (int i = 0; i < vN; i++) {
        result[i] = a[i] * b[i] + i * 0.5f;
    }
}

/* Helper for verification */
int verify_results(float *cpu, float *gpu, int N, float tolerance) {
    for (int i = 0; i < N; i++) {
        if (fabs(cpu[i] - gpu[i]) > tolerance) {
            printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                   i, cpu[i], gpu[i]);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes from command line or defaults */
    int N1 = (argc > 1) ? atoi(argv[1]) : 1024;
    int N2 = (argc > 2) ? atoi(argv[2]) : 2048;
    int N3 = (argc > 3) ? atoi(argv[3]) : 512;
    
    /* Allocate arrays with dynamic sizes */
    float *a1 = (float *)malloc(N1 * sizeof(float));
    float *b1 = (float *)malloc(N1 * sizeof(float));
    float *result1 = (float *)malloc(N1 * sizeof(float));
    float *cpu1 = (float *)malloc(N1 * sizeof(float));
    
    float *a2 = (float *)malloc(N2 * sizeof(float));
    float *b2 = (float *)malloc(N2 * sizeof(float));
    float *result2 = (float *)malloc(N2 * sizeof(float));
    
    float *a3 = (float *)malloc(N3 * sizeof(float));
    float *b3 = (float *)malloc(N3 * sizeof(float));
    float *result3 = (float *)malloc(N3 * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N1; i++) {
        a1[i] = i * 1.5f;
        b1[i] = i * 0.5f;
        cpu1[i] = a1[i] + b1[i] * 2.0f;  /* Expected result */
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = i * 2.0f;
        b2[i] = i * 0.25f;
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = i * 3.0f;
        b3[i] = i * 0.75f;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Primary SIMT pattern - called multiple times */
    compute_simt(N1, a1, b1, result1);
    
    /* Branch to create different control flow contexts */
    if (N1 > 500) {
        compute_simt(N1/2, a1, b1, result1);
    }
    
    /* Test 2: Nested construct pattern */
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Test 3: target simd pattern */
    compute_target_simd(N3, a3, b3, result3);
    
    /* Verification to prevent dead code elimination */
    int valid = verify_results(cpu1, result1, N1, 0.001f);
    
    if (valid) {
        printf("Test 1 passed: SIMT transformation verified\n");
        
        /* Additional computation to keep all paths alive */
        float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;
        for (int i = 0; i < N1; i++) sum1 += result1[i];
        for (int i = 0; i < N2; i++) sum2 += result2[i];
        for (int i = 0; i < N3; i++) sum3 += result3[i];
        
        printf("Result sums: %f, %f, %f\n", sum1, sum2, sum3);
    } else {
        printf("Test 1 failed\n");
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    return 0;
}
