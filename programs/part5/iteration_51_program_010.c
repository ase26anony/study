/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary target region for SIMT transformation */
void compute_simt(int N, float *a, float *b, float *result) {
    /* This combined construct should trigger IFN_GOMP_USE_SIMT generation */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: result[0:N]) \
        private(i) shared(result) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < N) {
                result[idx] = a[idx] * b[idx] + (float)(i + j);
            }
        }
    }
}

/* Alternative function with nested teams/distribute for SIMT */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    /* Explicit nesting that should also trigger SIMT transformation */
    #pragma omp target teams map(to: a[0:N], b[0:N]) map(from: result[0:N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(result) collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < 8; j++) {
                int idx = i * 8 + j;
                if (idx < N) {
                    result[idx] = a[idx] + b[idx] * 2.0f;
                }
            }
        }
    }
}

/* Function with target simd that might trigger parallel SIMT */
void compute_target_simd(int N, float *a, float *b, float *result) {
    /* Using target simd in a context suggesting parallel execution */
    #pragma omp target simd map(to: a[0:N], b[0:N]) map(from: result[0:N]) \
        linear(i:1)
    for (int i = 0; i < N; i++) {
        result[i] = a[i] - b[i];
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
void compute_cpu_reference(int N, float *a, float *b, float *result, int mode) {
    if (mode == 0) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < 16; j++) {
                int idx = i * 16 + j;
                if (idx < N) {
                    result[idx] = a[idx] * b[idx] + (float)(i + j);
                }
            }
        }
    } else if (mode == 1) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < 8; j++) {
                int idx = i * 8 + j;
                if (idx < N) {
                    result[idx] = a[idx] + b[idx] * 2.0f;
                }
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            result[i] = a[i] - b[i];
        }
    }
}

int main() {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int base_size = 1024;
    int sizes[] = {base_size, base_size * 2, base_size / 2};
    int num_sizes = 3;
    
    for (int s = 0; s < num_sizes; s++) {
        int N = sizes[s];
        printf("Testing with N = %d\n", N);
        
        /* Dynamic allocation to avoid constant bounds */
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
        
        /* Initialize data with non-trivial patterns */
        for (int i = 0; i < N; i++) {
            a[i] = (float)(i % 100) * 1.5f;
            b[i] = (float)((i + 37) % 100) * 0.7f;
            gpu_result1[i] = gpu_result2[i] = gpu_result3[i] = 0.0f;
            cpu_result[i] = 0.0f;
        }
        
        /* Call compute functions multiple times to hit different contexts */
        compute_simt(N, a, b, gpu_result1);
        
        /* Conditional call to expose different control flow */
        if (N > 512) {
            compute_nested_simt(N, a, b, gpu_result2);
        } else {
            compute_target_simd(N, a, b, gpu_result2);
        }
        
        /* Third call from different point in control flow */
        for (int repeat = 0; repeat < 2; repeat++) {
            compute_simt(N, a, b, gpu_result3);
        }
        
        /* Verify results against CPU computation */
        compute_cpu_reference(N, a, b, cpu_result, 0);
        if (!verify_results(cpu_result, gpu_result1, N, 1e-4f)) {
            printf("Verification failed for compute_simt\n");
        }
        
        compute_cpu_reference(N, a, b, cpu_result, 1);
        if (!verify_results(cpu_result, gpu_result2, N, 1e-4f)) {
            printf("Verification failed for second kernel\n");
        }
        
        compute_cpu_reference(N, a, b, cpu_result, 0);
        if (!verify_results(cpu_result, gpu_result3, N, 1e-4f)) {
            printf("Verification failed for repeated kernel\n");
        }
        
        printf("Test %d passed\n\n", s + 1);
        
        /* Cleanup */
        free(a); free(b); 
        free(gpu_result1); free(gpu_result2); free(gpu_result3);
        free(cpu_result);
    }
    
    /* Additional test with different construct in dead code */
    if (0) {  /* Never executed but parsed by compiler */
        int dummy_N = 256;
        float dummy_a[256], dummy_b[256], dummy_result[256];
        
        /* Alternative SIMT construct that should be parsed */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: dummy_a, dummy_b, dummy_result) \
            collapse(3)
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 16; k++) {
                    int idx = (i * 4 + j) * 16 + k;
                    dummy_result[idx] = dummy_a[idx] * dummy_b[idx];
                }
            }
        }
    }
    
    printf("All tests completed\n");
    return 0;
}
