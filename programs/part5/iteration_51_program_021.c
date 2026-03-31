/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT generation */
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
        num_teams(256) thread_limit(128)
    for (int i = 0; i < dynamic_N; i++) {
        result[i] = a[i] * b[i] + sinf((float)i);
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
            for (int j = 0; j < 16; j++) {
                int idx = i * 16 + j;
                if (idx < size) {
                    result[idx] = a[idx] * b[idx] + cosf((float)idx);
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int n = N;
    
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: result[0:n])
    {
        #pragma omp parallel for simd \
            private(i) shared(a, b, result) \
            schedule(static, 32)
        for (int i = 0; i < n; i++) {
            result[i] = a[i] * b[i] + tanf((float)i * 0.01f);
        }
    }
}

/* Helper function to verify results */
int verify_results(int N, float *cpu_result, float *gpu_result, float tolerance) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabsf(cpu_result[i] - gpu_result[i]) > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                       i, cpu_result[i], gpu_result[i]);
            }
        }
    }
    return errors;
}

/* CPU reference implementation */
void compute_cpu_reference(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        result[i] = a[i] * b[i] + sinf((float)i);
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int base_size = 1000;
    if (argc > 1) base_size = atoi(argv[1]);
    
    /* Create multiple sizes to test different contexts */
    int sizes[] = {base_size, base_size * 2, base_size / 2 + 1};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int test_case = 0; test_case < num_sizes; test_case++) {
        int N = sizes[test_case];
        printf("Test case %d: N = %d\n", test_case, N);
        
        /* Dynamic allocation prevents constant propagation */
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
            a[i] = (float)(i % 100) * 0.1f;
            b[i] = (float)((i + 37) % 100) * 0.2f;
        }
        
        /* Call from different contexts to affect current_function_decl usage */
        if (test_case % 2 == 0) {
            compute_simt(N, a, b, gpu_result);
        } else {
            compute_nested_simt(N, a, b, gpu_result);
        }
        
        /* Compute reference on CPU */
        compute_cpu_reference(N, a, b, cpu_result);
        
        /* Verify results */
        int errors = verify_results(N, cpu_result, gpu_result, 1e-4f);
        if (errors > 0) {
            printf("  FAILED: %d errors\n", errors);
        } else {
            printf("  PASSED\n");
        }
        
        /* Test alternative construct in dead code to expose compiler to syntax */
        if (0) {  /* Never executed, but parsed by compiler */
            float dummy_a[10], dummy_b[10], dummy_result[10];
            compute_target_simd(10, dummy_a, dummy_b, dummy_result);
            
            /* Additional variant with different clause ordering */
            #pragma omp target teams distribute parallel for simd \
                map(tofrom: dummy_result[0:10]) \
                simdlen(8) \
                private(i)
            for (int i = 0; i < 10; i++) {
                dummy_result[i] = 0.0f;
            }
        }
        
        free(a);
        free(b);
        free(gpu_result);
        free(cpu_result);
    }
    
    /* Additional test with collapse clause on triangular loop */
    {
        int N = 100;
        float *matrix = (float *)malloc(N * N * sizeof(float));
        
        #pragma omp target teams distribute parallel for simd \
            map(from: matrix[0:N*N]) \
            collapse(2) \
            private(i, j)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j <= i; j++) {  /* Non-rectangular but collapse(2) still valid */
                matrix[i * N + j] = (float)(i * j);
            }
        }
        
        free(matrix);
    }
    
    printf("All tests completed\n");
    return 0;
}
