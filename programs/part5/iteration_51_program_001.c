/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT generation */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static volatile int g_size = 0;

/* Function containing the primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Non-constant bounds to prevent loop elimination */
    int size = N + g_size;
    
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:size], b[0:size]) \
                map(from: result[0:size]) \
                private(i) shared(result) \
                simdlen(8)
    for (int i = 0; i < size; i++) {
        result[i] = a[i] * 2.0f + b[i];
    }
}

/* Alternative function with nested teams/distribute */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    int size = N + (g_size & 1); /* Ensure non-constant */
    
    #pragma omp target teams map(to: a[0:size], b[0:size]) \
                             map(from: result[0:size]) \
                             num_teams(8) thread_limit(64)
    {
        #pragma omp distribute parallel for simd \
                    private(i) shared(a,b,result) \
                    collapse(1)
        for (int i = 0; i < size; i++) {
            result[i] = a[i] + b[i] * 3.0f;
        }
    }
}

/* Function with 2D collapsed loop for more complex transformation */
void compute_collapsed_simt(int N, float *a, float *result) {
    int dim = (N > 16) ? 16 : N;
    dim += g_size & 3; /* Non-constant adjustment */
    
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:dim*dim]) \
                map(from: result[0:dim*dim]) \
                collapse(2) \
                private(i,j) shared(a,result)
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            int idx = i * dim + j;
            result[idx] = a[idx] * (i + j + 1);
        }
    }
}

/* Function with target simd and parallel for comparison */
void compute_target_simd(int N, float *a, float *result) {
    int size = N + (g_size & 7);
    
    /* This may trigger different SIMT transformation path */
    #pragma omp target simd map(to: a[0:size]) map(from: result[0:size]) \
                 private(i) linear(i:1)
    for (int i = 0; i < size; i++) {
        result[i] = a[i] * a[i];
    }
}

/* Verification function */
int verify_results(float *cpu, float *gpu, int size, float tolerance) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        float diff = cpu[i] - gpu[i];
        if (diff < -tolerance || diff > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at %d: CPU=%f, GPU=%f\n", i, cpu[i], gpu[i]);
            }
        }
    }
    return errors;
}

int main(int argc, char *argv[]) {
    /* Use command line or non-constant sizes */
    int base_size = (argc > 1) ? atoi(argv[1]) : 1024;
    if (base_size < 64) base_size = 64;
    
    /* Create multiple different sizes to test various paths */
    int sizes[] = {base_size, base_size + 128, base_size * 2};
    int num_tests = sizeof(sizes) / sizeof(sizes[0]);
    
    int total_errors = 0;
    
    for (int test = 0; test < num_tests; test++) {
        int N = sizes[test];
        printf("Test %d with size %d\n", test, N);
        
        /* Dynamic allocation prevents compile-time optimization */
        float *a = (float *)malloc(N * sizeof(float));
        float *b = (float *)malloc(N * sizeof(float));
        float *result_gpu = (float *)malloc(N * sizeof(float));
        float *result_cpu = (float *)malloc(N * sizeof(float));
        
        if (!a || !b || !result_gpu || !result_cpu) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        
        /* Initialize with pattern */
        for (int i = 0; i < N; i++) {
            a[i] = (float)(i % 37) * 0.1f;
            b[i] = (float)(i % 23) * 0.2f;
        }
        
        /* Call from different contexts to affect current_function_decl usage */
        if (test == 0) {
            compute_simt(N, a, b, result_gpu);
        } else if (test == 1) {
            compute_nested_simt(N, a, b, result_gpu);
        } else {
            compute_collapsed_simt(N, a, result_gpu);
        }
        
        /* Also call target_simd variant in conditional to expose to compiler */
        if (N % 2 == 0) {
            compute_target_simd(N, a, result_cpu);
        }
        
        /* CPU verification computation */
        for (int i = 0; i < N; i++) {
            if (test == 2) {
                /* For collapsed test */
                int dim = (N > 16) ? 16 : N;
                dim += g_size & 3;
                int row = i / dim;
                int col = i % dim;
                result_cpu[i] = a[i] * (row + col + 1);
            } else {
                result_cpu[i] = a[i] * 2.0f + b[i];
            }
        }
        
        /* Verify results */
        int errors = verify_results(result_cpu, result_gpu, N, 0.001f);
        if (errors > 0) {
            printf("  Found %d errors\n", errors);
            total_errors += errors;
        } else {
            printf("  Results match\n");
        }
        
        /* Free memory */
        free(a);
        free(b);
        free(result_gpu);
        free(result_cpu);
    }
    
    /* Additional test with different call pattern */
    {
        int N = base_size + 256;
        float *temp_a = (float *)malloc(N * sizeof(float));
        float *temp_res = (float *)malloc(N * sizeof(float));
        
        if (temp_a && temp_res) {
            for (int i = 0; i < N; i++) {
                temp_a[i] = (float)i;
            }
            
            /* Call from within a loop with varying conditions */
            for (int iter = 0; iter < 3; iter++) {
                if (iter == 1) {
                    compute_target_simd(N, temp_a, temp_res);
                } else {
                    compute_simt(N, temp_a, temp_a, temp_res);
                }
            }
            
            free(temp_a);
            free(temp_res);
        }
    }
    
    printf("\nTotal errors across all tests: %d\n", total_errors);
    
    return total_errors > 0 ? 1 : 0;
}
