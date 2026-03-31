/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT generation */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary target region for SIMT transformation */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    /* Primary target region with teams distribute parallel for simd */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) \
        num_teams(256) thread_limit(128)
    for (int i = 0; i < dynamic_N; i++) {
        result[i] = a[i] + b[i] * 2.0f;
    }
}

/* Alternative function with nested constructs */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int rows = N / 16;
    volatile int cols = 16;
    
    /* Nested teams and distribute parallel for simd */
    #pragma omp target teams map(to: a[0:N], b[0:N]) map(from: result[0:N])
    {
        #pragma omp distribute parallel for simd collapse(2) \
            private(i,j) shared(a,b,result)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                result[idx] = a[idx] - b[idx];
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    /* Create parallel region that might trigger SIMT transformation */
    #pragma omp target map(to: a[0:size], b[0:size]) map(from: result[0:size])
    {
        #pragma omp parallel
        {
            #pragma omp simd
            for (int i = 0; i < size; i++) {
                result[i] = a[i] * b[i];
            }
        }
    }
}

/* Helper function to verify results */
int verify_results(float *cpu_result, float *gpu_result, int N, float tolerance) {
    for (int i = 0; i < N; i++) {
        if (fabs(cpu_result[i] - gpu_result[i]) > tolerance) {
            fprintf(stderr, "Mismatch at index %d: CPU=%f, GPU=%f\n", 
                    i, cpu_result[i], gpu_result[i]);
            return 0;
        }
    }
    return 1;
}

/* CPU reference implementation */
void cpu_compute(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        result[i] = a[i] + b[i] * 2.0f;
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant size to prevent compile-time optimization */
    int N = 1024;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1024;
    }
    
    /* Dynamic allocation to avoid constant propagation */
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *gpu_result = (float *)malloc(N * sizeof(float));
    float *cpu_result = (float *)malloc(N * sizeof(float));
    
    if (!a || !b || !gpu_result || !cpu_result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
    }
    
    printf("Testing SIMT transformation with N = %d\n", N);
    
    /* Multiple invocations to hit different contexts */
    for (int iter = 0; iter < 3; iter++) {
        printf("Iteration %d:\n", iter + 1);
        
        /* Call from different control flow paths */
        if (iter % 2 == 0) {
            /* Primary SIMT transformation path */
            compute_simt(N, a, b, gpu_result);
            
            /* CPU verification */
            cpu_compute(N, a, b, cpu_result);
            if (verify_results(cpu_result, gpu_result, N, 1e-6f)) {
                printf("  compute_simt: PASS\n");
            } else {
                printf("  compute_simt: FAIL\n");
            }
        } else {
            /* Alternative nested construct */
            compute_nested_simt(N, a, b, gpu_result);
            
            /* Simple CPU verification for this pattern */
            for (int i = 0; i < N; i++) {
                cpu_result[i] = a[i] - b[i];
            }
            if (verify_results(cpu_result, gpu_result, N, 1e-6f)) {
                printf("  compute_nested_simt: PASS\n");
            } else {
                printf("  compute_nested_simt: FAIL\n");
            }
        }
        
        /* Conditional block with alternative construct (always false at runtime) */
        if (0) {  /* Never executed but present in IR */
            compute_target_simd(N, a, b, gpu_result);
        }
    }
    
    /* Additional test with different data size */
    int half_N = N / 2;
    if (half_N > 0) {
        printf("\nTesting with half size (%d):\n", half_N);
        compute_simt(half_N, a, b, gpu_result);
        cpu_compute(half_N, a, b, cpu_result);
        if (verify_results(cpu_result, gpu_result, half_N, 1e-6f)) {
            printf("  Half-size test: PASS\n");
        } else {
            printf("  Half-size test: FAIL\n");
        }
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(gpu_result);
    free(cpu_result);
    
    printf("\nAll tests completed\n");
    return 0;
}
