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
        private(i) shared(result) \
        collapse(2)
    for (int i = 0; i < dynamic_N; i++) {
        for (int j = 0; j < 4; j++) {  /* Inner loop for collapse(2) */
            int idx = i * 4 + j;
            if (idx < dynamic_N) {
                result[idx] = a[idx] * b[idx] + (float)j;
            }
        }
    }
}

/* Alternative function with nested teams/distribute construct */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    #pragma omp target teams map(to: a[0:size], b[0:size]) map(from: result[0:size])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(result) \
            simdlen(8)
        for (int i = 0; i < size; i++) {
            result[i] = a[i] + b[i] * 2.0f;
        }
    }
}

/* Function with target simd and parallel execution hint */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int n = N;
    
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: result[0:n])
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        result[i] = a[i] - b[i];
    }
}

/* Verification function */
int verify_results(int N, float *cpu_result, float *gpu_result) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(cpu_result[i] - gpu_result[i]) > 1e-6) {
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
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx < N) {
                result[idx] = a[idx] * b[idx] + (float)j;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int base_size = 1000;
    if (argc > 1) base_size = atoi(argv[1]);
    
    /* Multiple different sizes to test different contexts */
    int sizes[] = {base_size, base_size * 2, base_size / 2};
    int num_sizes = 3;
    
    for (int s = 0; s < num_sizes; s++) {
        int N = sizes[s];
        if (N < 10) N = 10;  /* Ensure minimum size */
        
        printf("Testing with N = %d\n", N);
        
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
            a[i] = (float)i * 1.5f;
            b[i] = (float)(N - i) * 0.5f;
        }
        
        /* Call from different contexts to influence current_function_decl usage */
        if (s == 0) {
            /* First call - primary SIMT transformation */
            compute_simt(N, a, b, gpu_result);
        } else if (s == 1) {
            /* Second call - nested construct */
            compute_nested_simt(N, a, b, gpu_result);
        } else {
            /* Third call - target simd variant */
            compute_target_simd(N, a, b, gpu_result);
        }
        
        /* Compute reference on CPU */
        cpu_compute(N, a, b, cpu_result);
        
        /* Verify results - prevents dead code elimination */
        int errors = verify_results(N, cpu_result, gpu_result);
        
        if (errors == 0) {
            printf("  Test passed: %d elements verified\n", N);
        } else {
            printf("  Test failed: %d errors found\n", errors);
        }
        
        /* Free memory */
        free(a);
        free(b);
        free(gpu_result);
        free(cpu_result);
    }
    
    /* Additional test with conditional execution */
    printf("\nTesting conditional execution path:\n");
    {
        int N = 500;
        float *a = (float *)malloc(N * sizeof(float));
        float *b = (float *)malloc(N * sizeof(float));
        float *result = (float *)malloc(N * sizeof(float));
        
        for (int i = 0; i < N; i++) {
            a[i] = (float)i;
            b[i] = 2.0f;
        }
        
        /* Conditional that might affect code generation */
        int use_simt = 1;
        if (use_simt) {
            compute_simt(N, a, b, result);
        }
        
        /* Use result to prevent optimization */
        float sum = 0.0f;
        for (int i = 0; i < N; i++) {
            sum += result[i];
        }
        printf("Conditional test sum: %f\n", sum);
        
        free(a);
        free(b);
        free(result);
    }
    
    return 0;
}
