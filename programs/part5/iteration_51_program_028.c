/* Test program to trigger SIMT transformation in GCC omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT target region */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: a[0:dynamic_N], b[0:dynamic_N], result[0:dynamic_N]) \
                private(i) shared(result) collapse(2)
    for (int i = 0; i < dynamic_N; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < dynamic_N) {
                result[idx] = a[idx] * b[idx] + (float)j;
            }
        }
    }
}

/* Alternative function with nested teams/distribute construct */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int vol_N = N;
    
    #pragma omp target teams map(tofrom: a[0:vol_N], b[0:vol_N], result[0:vol_N])
    {
        #pragma omp distribute parallel for simd \
                    private(i) shared(a, b, result) collapse(2)
        for (int i = 0; i < vol_N; i++) {
            for (int j = 0; j < 8; j++) {
                int idx = i * 8 + j;
                if (idx < vol_N) {
                    result[idx] = a[idx] + b[idx] * (float)(i + j);
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    /* Create parallel context that might trigger SIMT transformation */
    #pragma omp target map(tofrom: a[0:size], b[0:size], result[0:size])
    {
        #pragma omp parallel
        {
            #pragma omp for simd private(i) nowait
            for (int i = 0; i < size; i++) {
                result[i] = a[i] - b[i];
            }
        }
    }
}

/* Helper function to verify results */
int verify_results(float *cpu_result, float *gpu_result, int N, float tolerance) {
    for (int i = 0; i < N; i++) {
        if (fabsf(cpu_result[i] - gpu_result[i]) > tolerance) {
            printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                   i, cpu_result[i], gpu_result[i]);
            return 0;
        }
    }
    return 1;
}

/* CPU reference implementation */
void compute_cpu_reference(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < N) {
                result[idx] = a[idx] * b[idx] + (float)j;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int base_size = (argc > 1) ? atoi(argv[1]) : 1024;
    if (base_size < 64) base_size = 64;
    
    /* Test multiple sizes to hit different code paths */
    int sizes[] = {base_size, base_size * 2, base_size / 2};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int test_case = 0; test_case < num_sizes; test_case++) {
        int N = sizes[test_case];
        printf("Test case %d: N = %d\n", test_case, N);
        
        /* Dynamic allocation prevents compile-time analysis */
        float *a = (float *)malloc(N * sizeof(float));
        float *b = (float *)malloc(N * sizeof(float));
        float *gpu_result = (float *)malloc(N * sizeof(float));
        float *cpu_result = (float *)malloc(N * sizeof(float));
        
        if (!a || !b || !gpu_result || !cpu_result) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        
        /* Initialize with non-constant pattern */
        for (int i = 0; i < N; i++) {
            a[i] = (float)(i % 37) * 0.1f;
            b[i] = (float)(i % 23) * 0.2f;
            gpu_result[i] = 0.0f;
            cpu_result[i] = 0.0f;
        }
        
        /* Call from different contexts to affect current_function_decl usage */
        if (test_case % 2 == 0) {
            compute_simt(N, a, b, gpu_result);
        } else {
            compute_nested_simt(N, a, b, gpu_result);
        }
        
        /* Compute reference on CPU */
        compute_cpu_reference(N, a, b, cpu_result);
        
        /* Verify results - prevents dead code elimination */
        if (verify_results(cpu_result, gpu_result, N, 1e-5f)) {
            printf("  Test passed\n");
        } else {
            printf("  Test failed\n");
        }
        
        /* Try alternative construct in conditional block */
        if (test_case == 1) {
            /* This block should still be processed by the compiler */
            #pragma omp target teams distribute parallel for simd \
                        map(tofrom: a[0:N]) if(0)
            for (int i = 0; i < N; i++) {
                a[i] *= 2.0f;  /* Dead code, but still parsed */
            }
        }
        
        free(a);
        free(b);
        free(gpu_result);
        free(cpu_result);
    }
    
    /* Additional test with target simd in parallel context */
    printf("\nTesting target simd with parallel context:\n");
    {
        int N = 512;
        float *x = (float *)malloc(N * sizeof(float));
        float *y = (float *)malloc(N * sizeof(float));
        float *z = (float *)malloc(N * sizeof(float));
        
        for (int i = 0; i < N; i++) {
            x[i] = (float)i;
            y[i] = (float)(N - i);
            z[i] = 0.0f;
        }
        
        compute_target_simd(N, x, y, z);
        
        /* Simple verification */
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (fabsf(z[i] - (x[i] - y[i])) > 1e-5f) {
                errors++;
            }
        }
        printf("  Errors in target simd test: %d\n", errors);
        
        free(x);
        free(y);
        free(z);
    }
    
    return 0;
}
