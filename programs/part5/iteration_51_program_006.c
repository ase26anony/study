/* Test program to trigger SIMT transformation in GCC's OpenMP lowering */
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
        private(i) shared(result) \
        num_teams(256) thread_limit(64)
    for (int i = 0; i < dynamic_N; i++) {
        result[i] = a[i] * b[i] + (float)i;
    }
}

/* Alternative function with nested teams/distribute */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    #pragma omp target teams map(to: a[0:size], b[0:size]) map(from: result[0:size])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a, b, result) \
            collapse(2)
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < size) {
                    result[idx] = a[idx] * 2.0f + b[idx] * 3.0f;
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int n = N;
    
    /* Create a parallel region that might influence SIMT transformation */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp target simd map(to: a[0:n], b[0:n]) map(from: result[0:n])
            for (int i = 0; i < n; i++) {
                result[i] = a[i] - b[i];
            }
        }
    }
}

/* Helper function to verify results */
int verify_results(float *cpu, float *gpu, int N, float tolerance) {
    for (int i = 0; i < N; i++) {
        if (fabs(cpu[i] - gpu[i]) > tolerance) {
            printf("Mismatch at index %d: CPU=%f, GPU=%f\n", i, cpu[i], gpu[i]);
            return 0;
        }
    }
    return 1;
}

/* Main function orchestrating multiple SIMT patterns */
int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent optimization */
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
        a1[i] = (float)(i % 100);
        b1[i] = (float)((i + 1) % 100);
        cpu1[i] = a1[i] * b1[i] + (float)i;  /* CPU reference */
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)(i % 50);
        b2[i] = (float)((i * 2) % 50);
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)(i % 200);
        b3[i] = (float)((i + 3) % 200);
    }
    
    /* Pattern 1: Primary SIMT transformation target */
    printf("Running primary SIMT pattern...\n");
    compute_simt(N1, a1, b1, result1);
    
    /* Verify results */
    if (verify_results(cpu1, result1, N1, 1e-6f)) {
        printf("Pattern 1 verification passed\n");
    }
    
    /* Pattern 2: Nested teams/distribute with collapse */
    printf("Running nested SIMT pattern...\n");
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Pattern 3: Conditional execution to vary context */
    if (N3 > 100) {
        printf("Running target simd pattern...\n");
        compute_target_simd(N3, a3, b3, result3);
    }
    
    /* Additional call with different size to hit different paths */
    if (argc > 4) {
        int N4 = atoi(argv[4]);
        float *a4 = (float *)malloc(N4 * sizeof(float));
        float *b4 = (float *)malloc(N4 * sizeof(float));
        float *result4 = (float *)malloc(N4 * sizeof(float));
        
        for (int i = 0; i < N4; i++) {
            a4[i] = (float)i;
            b4[i] = (float)(N4 - i);
        }
        
        compute_simt(N4, a4, b4, result4);
        
        free(a4);
        free(b4);
        free(result4);
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    printf("All patterns executed\n");
    return 0;
}
