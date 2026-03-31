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
        result[i] = a[i] * b[i] + (float)i;
    }
}

/* Alternative function with nested constructs */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    /* Explicitly nested teams and distribute parallel for simd */
    #pragma omp target map(to: a[0:size], b[0:size]) map(from: result[0:size])
    {
        #pragma omp teams num_teams(8)
        {
            #pragma omp distribute parallel for simd \
                private(i) shared(a,b,result) \
                collapse(2)
            for (int i = 0; i < size/2; i++) {
                for (int j = 0; j < 2; j++) {
                    int idx = i*2 + j;
                    if (idx < size) {
                        result[idx] = a[idx] + b[idx] * (float)idx;
                    }
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int n = N;
    
    /* target simd in a context that suggests parallel execution */
    #pragma omp target teams distribute simd \
        map(to: a[0:n], b[0:n]) \
        map(from: result[0:n]) \
        private(i) \
        num_teams(2)
    for (int i = 0; i < n; i++) {
        result[i] = (a[i] - b[i]) * (float)i;
    }
}

/* Helper function to verify results */
int verify_results(int N, float *cpu_result, float *gpu_result, float tolerance) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float diff = cpu_result[i] - gpu_result[i];
        if (diff < -tolerance || diff > tolerance) {
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
        result[i] = a[i] * b[i] + (float)i;
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant size to prevent compile-time optimization */
    int base_size = 1024;
    if (argc > 1) {
        base_size = atoi(argv[1]);
    }
    
    /* Create multiple test sizes to hit different code paths */
    int sizes[] = {base_size, base_size * 2, base_size / 2};
    int num_tests = sizeof(sizes) / sizeof(sizes[0]);
    
    int total_errors = 0;
    
    for (int test = 0; test < num_tests; test++) {
        int N = sizes[test];
        if (N <= 0) N = 256;  /* Ensure valid size */
        
        printf("Test %d: N = %d\n", test + 1, N);
        
        /* Allocate arrays */
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
            a[i] = (float)(i % 100);
            b[i] = (float)((i + 1) % 100);
        }
        
        /* Compute CPU reference */
        compute_cpu_reference(N, a, b, cpu_result);
        
        /* Call SIMT function multiple times from different contexts */
        if (test % 2 == 0) {
            /* First call path */
            compute_simt(N, a, b, gpu_result);
        } else {
            /* Alternative call path */
            compute_nested_simt(N, a, b, gpu_result);
        }
        
        /* Verify results */
        int errors = verify_results(N, cpu_result, gpu_result, 0.001f);
        total_errors += errors;
        
        if (errors == 0) {
            printf("  Test %d passed\n", test + 1);
        } else {
            printf("  Test %d failed with %d errors\n", test + 1, errors);
        }
        
        /* Additional call to target simd variant (conditionally executed) */
        if (test == 1) {
            compute_target_simd(N, a, b, gpu_result);
            errors = verify_results(N, cpu_result, gpu_result, 0.001f);
            if (errors > 0) {
                printf("  Target SIMD variant had %d errors\n", errors);
            }
        }
        
        /* Free memory */
        free(a);
        free(b);
        free(gpu_result);
        free(cpu_result);
    }
    
    /* Final test with collapse clause on larger nested loop */
    printf("\nFinal test with collapse(2) on larger problem:\n");
    {
        int N = 512;
        int M = 4;
        float *matrix = (float *)malloc(N * M * sizeof(float));
        float *vector = (float *)malloc(M * sizeof(float));
        float *output = (float *)malloc(N * sizeof(float));
        
        for (int i = 0; i < N * M; i++) matrix[i] = (float)i;
        for (int j = 0; j < M; j++) vector[j] = (float)(j + 1);
        
        volatile int n = N, m = M;
        
        /* Complex collapsed loop that should trigger SIMT transformation */
        #pragma omp target teams distribute parallel for simd \
            map(to: matrix[0:n*m], vector[0:m]) \
            map(from: output[0:n]) \
            collapse(2) \
            num_teams(8)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                output[i] += matrix[i * m + j] * vector[j];
            }
        }
        
        free(matrix);
        free(vector);
        free(output);
    }
    
    if (total_errors == 0) {
        printf("\nAll tests passed successfully!\n");
        return 0;
    } else {
        printf("\nTotal errors across all tests: %d\n", total_errors);
        return 1;
    }
}
