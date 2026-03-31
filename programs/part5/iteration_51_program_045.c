/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT target region */
void compute_simt(int N, float *a, float *b, float *result) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: result[0:N]) \
        private(i) shared(a, b, result) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < N) {
                result[idx] = a[idx] * b[idx] + (float)j;
            }
        }
    }
}

/* Alternative function with nested teams/distribute */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    #pragma omp target teams map(to: a[0:N], b[0:N]) map(from: result[0:N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(a, b, result)
        for (int i = 0; i < N; i++) {
            result[i] = a[i] + b[i] * 2.0f;
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    #pragma omp target map(to: a[0:N], b[0:N]) map(from: result[0:N])
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        result[i] = a[i] - b[i];
    }
}

/* Helper function to initialize arrays */
void init_arrays(int N, float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.5f;
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
                printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                       i, cpu_result[i], gpu_result[i]);
            }
        }
    }
    return errors;
}

/* CPU reference implementation */
void cpu_compute(int N, float *a, float *b, float *result) {
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
    /* Use non-constant sizes to prevent constant folding */
    int base_size = 1024;
    if (argc > 1) {
        base_size = atoi(argv[1]);
    }
    
    /* Create multiple test sizes to stress different paths */
    int sizes[] = {base_size, base_size * 2, base_size / 2};
    int num_sizes = 3;
    
    for (int s = 0; s < num_sizes; s++) {
        int N = sizes[s];
        if (N <= 0) N = 256;
        
        printf("Testing with N = %d\n", N);
        
        /* Dynamic allocation prevents compile-time optimization */
        float *a = (float *)malloc(N * sizeof(float));
        float *b = (float *)malloc(N * sizeof(float));
        float *gpu_result = (float *)malloc(N * sizeof(float));
        float *cpu_result = (float *)malloc(N * sizeof(float));
        
        if (!a || !b || !gpu_result || !cpu_result) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        
        init_arrays(N, a, b);
        
        /* Test 1: Primary SIMT construct */
        printf("  Testing primary SIMT construct...\n");
        compute_simt(N, a, b, gpu_result);
        
        /* CPU verification */
        cpu_compute(N, a, b, cpu_result);
        int errors = verify_results(N, cpu_result, gpu_result, 0.001f);
        printf("    Errors: %d\n", errors);
        
        /* Test 2: Nested construct (call from different context) */
        if (s % 2 == 0) {
            printf("  Testing nested construct...\n");
            compute_nested_simt(N, a, b, gpu_result);
            
            /* Simple CPU verification for nested version */
            errors = 0;
            for (int i = 0; i < N; i++) {
                float expected = a[i] + b[i] * 2.0f;
                float diff = gpu_result[i] - expected;
                if (diff < -0.001f || diff > 0.001f) {
                    errors++;
                }
            }
            printf("    Errors: %d\n", errors);
        }
        
        /* Test 3: Conditional execution path */
        volatile int use_target_simd = 1; /* volatile prevents optimization */
        if (use_target_simd) {
            printf("  Testing target simd construct...\n");
            compute_target_simd(N, a, b, gpu_result);
            
            /* Verification */
            errors = 0;
            for (int i = 0; i < N; i++) {
                float expected = a[i] - b[i];
                float diff = gpu_result[i] - expected;
                if (diff < -0.001f || diff > 0.001f) {
                    errors++;
                }
            }
            printf("    Errors: %d\n", errors);
        }
        
        free(a);
        free(b);
        free(gpu_result);
        free(cpu_result);
    }
    
    /* Additional test with different loop structure */
    printf("\nTesting with 2D collapse...\n");
    {
        int N1 = 64, N2 = 32;
        int total = N1 * N2;
        float *a = (float *)malloc(total * sizeof(float));
        float *b = (float *)malloc(total * sizeof(float));
        float *result = (float *)malloc(total * sizeof(float));
        
        for (int i = 0; i < total; i++) {
            a[i] = (float)i;
            b[i] = (float)(total - i);
        }
        
        /* Complex collapse clause */
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:total], b[0:total]) map(from: result[0:total]) \
            collapse(2)
        for (int i = 0; i < N1; i++) {
            for (int j = 0; j < N2; j++) {
                int idx = i * N2 + j;
                result[idx] = a[idx] * 2.0f + b[idx] * 0.5f;
            }
        }
        
        /* Verify */
        int errors = 0;
        for (int i = 0; i < total; i++) {
            float expected = a[i] * 2.0f + b[i] * 0.5f;
            if (result[i] != expected) errors++;
        }
        printf("  2D collapse errors: %d\n", errors);
        
        free(a);
        free(b);
        free(result);
    }
    
    return 0;
}
