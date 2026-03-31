/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    /* Primary target region designed for SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) \
        num_teams(256) thread_limit(64)
    for (int i = 0; i < dynamic_N; i++) {
        result[i] = a[i] * b[i] + a[i] / (b[i] + 1.0f);
    }
}

/* Alternative function with nested teams/distribute for SIMT */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int dyn_N = N;
    
    /* Nested construct that should also trigger SIMT */
    #pragma omp target teams map(to: a[0:dyn_N], b[0:dyn_N]) \
                             map(from: result[0:dyn_N]) \
                             num_teams(128)
    {
        #pragma omp distribute parallel for simd \
                private(i) shared(a,b,result) \
                collapse(2)
        for (int i = 0; i < dyn_N; i++) {
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < dyn_N) {
                    result[idx] = a[idx] + b[idx] * (j + 1);
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int vN = N;
    
    /* target simd in a context that suggests parallel GPU execution */
    #pragma omp target map(to: a[0:vN], b[0:vN]) \
                       map(from: result[0:vN]) \
                       device(0)
    #pragma omp parallel for simd private(i) \
                shared(a,b,result) schedule(static, 32)
    for (int i = 0; i < vN; i++) {
        result[i] = (a[i] * a[i]) - (b[i] * b[i]);
    }
}

/* Helper function to initialize arrays */
void init_arrays(int N, float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100) * 1.5f;
        b[i] = (float)((i + 1) % 100) * 0.7f;
    }
}

/* Verification function */
int verify_results(int N, float *cpu_result, float *gpu_result, float tolerance) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float diff = cpu_result[i] - gpu_result[i];
        if (diff < -tolerance || diff > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at %d: CPU=%.6f, GPU=%.6f\n", 
                       i, cpu_result[i], gpu_result[i]);
            }
        }
    }
    return errors;
}

/* CPU reference computation */
void cpu_compute(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        result[i] = a[i] * b[i] + a[i] / (b[i] + 1.0f);
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int base_size = 10000;
    if (argc > 1) base_size = atoi(argv[1]);
    
    /* Create multiple sizes to test different contexts */
    int sizes[] = {base_size, base_size * 2, base_size / 2};
    int num_sizes = 3;
    
    for (int s = 0; s < num_sizes; s++) {
        int N = sizes[s];
        printf("Testing with N = %d\n", N);
        
        /* Dynamic allocation prevents compile-time analysis */
        float *a = (float *)malloc(N * sizeof(float));
        float *b = (float *)malloc(N * sizeof(float));
        float *gpu_result = (float *)malloc(N * sizeof(float));
        float *cpu_result = (float *)malloc(N * sizeof(float));
        
        if (!a || !b || !gpu_result || !cpu_result) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        
        init_arrays(N, a, b);
        
        /* Multiple invocations to hit different contexts */
        for (int iter = 0; iter < 3; iter++) {
            /* Call from different control flow paths */
            if (iter % 2 == 0) {
                compute_simt(N, a, b, gpu_result);
            } else {
                compute_nested_simt(N, a, b, gpu_result);
            }
            
            /* CPU verification to prevent dead code elimination */
            cpu_compute(N, a, b, cpu_result);
            
            int errors = verify_results(N, cpu_result, gpu_result, 1e-5f);
            if (errors > 0) {
                printf("Iteration %d: %d errors found\n", iter, errors);
            } else {
                printf("Iteration %d: PASS\n", iter);
            }
        }
        
        /* Test alternative construct in conditional context */
        if (s == 1) {  /* Only for middle size */
            compute_target_simd(N, a, b, gpu_result);
            cpu_compute(N, a, b, cpu_result);
            int errors = verify_results(N, cpu_result, gpu_result, 1e-5f);
            printf("Target SIMD test: %d errors\n", errors);
        }
        
        free(a);
        free(b);
        free(gpu_result);
        free(cpu_result);
    }
    
    /* Additional test with collapse clause */
    printf("\nTesting with collapse clause:\n");
    int N = 512;
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *result = (float *)malloc(N * sizeof(float));
    
    init_arrays(N, a, b);
    
    /* Complex loop structure with collapse */
    volatile int dim1 = 16, dim2 = 32;
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: result[0:N]) \
        collapse(2) private(i,j) shared(a,b,result)
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            int idx = i * dim2 + j;
            if (idx < N) {
                result[idx] = a[idx] * 2.0f + b[idx] * 3.0f;
            }
        }
    }
    
    /* Verify */
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += result[i];
    }
    printf("Collapse test sum: %.2f\n", sum);
    
    free(a);
    free(b);
    free(result);
    
    return 0;
}
