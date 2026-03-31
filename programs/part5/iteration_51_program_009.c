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
        private(i) shared(result) \
        collapse(2) num_teams(128) thread_limit(64)
    for (int i = 0; i < dynamic_N; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < dynamic_N) {
                result[idx] = a[idx] * b[idx] + (float)(i + j);
            }
        }
    }
}

/* Alternative function with nested teams/distribute construct */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int vol_N = N;
    
    #pragma omp target teams map(to: a[0:vol_N], b[0:vol_N]) map(from: result[0:vol_N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a,b,result) \
            collapse(2)
        for (int i = 0; i < vol_N; i++) {
            for (int j = 0; j < 8; j++) {
                int idx = i * 8 + j;
                if (idx < vol_N) {
                    result[idx] = a[idx] / (b[idx] + 1.0f) + (float)(i * j);
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    /* Create context that suggests parallel GPU execution */
    #pragma omp target map(to: a[0:size], b[0:size]) map(from: result[0:size])
    {
        #pragma omp parallel
        {
            #pragma omp simd private(i)
            for (int i = 0; i < size; i++) {
                result[i] = a[i] + b[i] * 2.0f;
            }
        }
    }
}

/* Helper function for verification */
void verify_results(float *cpu_result, float *gpu_result, int N, const char *test_name) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(cpu_result[i] - gpu_result[i]) > 1e-6f) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at %d: CPU=%f, GPU=%f\n", 
                       i, cpu_result[i], gpu_result[i]);
            }
        }
    }
    printf("%s: %d errors out of %d elements\n", test_name, errors, N);
}

/* CPU reference implementation */
void cpu_compute(int N, float *a, float *b, float *result, int mode) {
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
                    result[idx] = a[idx] / (b[idx] + 1.0f) + (float)(i * j);
                }
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            result[i] = a[i] + b[i] * 2.0f;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int base_size = 1024;
    if (argc > 1) base_size = atoi(argv[1]);
    
    /* Create multiple sizes to test different contexts */
    int sizes[] = {base_size, base_size * 2, base_size / 2};
    int num_tests = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int test = 0; test < num_tests; test++) {
        int N = sizes[test];
        if (N < 16) N = 16;
        
        printf("\n=== Test %d with N=%d ===\n", test, N);
        
        /* Dynamic allocation prevents compile-time analysis */
        float *a = (float*)malloc(N * sizeof(float));
        float *b = (float*)malloc(N * sizeof(float));
        float *gpu_result = (float*)malloc(N * sizeof(float));
        float *cpu_result = (float*)malloc(N * sizeof(float));
        
        /* Initialize data */
        for (int i = 0; i < N; i++) {
            a[i] = (float)(i % 100) * 0.1f;
            b[i] = (float)((i + 1) % 100) * 0.2f;
        }
        
        /* Test 1: Primary SIMT transformation target */
        printf("Testing compute_simt...\n");
        compute_simt(N, a, b, gpu_result);
        cpu_compute(N, a, b, cpu_result, 0);
        verify_results(cpu_result, gpu_result, N, "compute_simt");
        
        /* Test 2: Nested construct */
        printf("Testing compute_nested_simt...\n");
        compute_nested_simt(N, a, b, gpu_result);
        cpu_compute(N, a, b, cpu_result, 1);
        verify_results(cpu_result, gpu_result, N, "compute_nested_simt");
        
        /* Test 3: target simd with parallel region */
        if (test == 0) {  /* Only test once to vary execution flow */
            printf("Testing compute_target_simd...\n");
            compute_target_simd(N, a, b, gpu_result);
            cpu_compute(N, a, b, cpu_result, 2);
            verify_results(cpu_result, gpu_result, N, "compute_target_simd");
        }
        
        /* Cleanup */
        free(a);
        free(b);
        free(gpu_result);
        free(cpu_result);
    }
    
    /* Additional test with conditional execution to affect context */
    printf("\n=== Conditional execution test ===\n");
    {
        int N = 512;
        float *a = (float*)malloc(N * sizeof(float));
        float *b = (float*)malloc(N * sizeof(float));
        float *result = (float*)malloc(N * sizeof(float));
        
        for (int i = 0; i < N; i++) {
            a[i] = (float)i;
            b[i] = (float)(N - i);
        }
        
        /* Conditional that might affect current_function_decl context */
        if (argc > 2) {
            compute_simt(N, a, b, result);
        } else {
            compute_nested_simt(N, a, b, result);
        }
        
        /* Force usage of result to prevent dead code elimination */
        float sum = 0.0f;
        for (int i = 0; i < N; i++) {
            sum += result[i];
        }
        printf("Result checksum: %f\n", sum);
        
        free(a);
        free(b);
        free(result);
    }
    
    return 0;
}
