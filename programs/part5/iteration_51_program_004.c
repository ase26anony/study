/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024
#define M 512

/* Function containing the primary SIMT transformation target */
void compute_simt(int n, int m, float *a, float *b, float *c) {
    int i, j;
    
    /* Primary target: teams distribute parallel for simd with collapse */
    #pragma omp target teams distribute parallel for simd \
                collapse(2) map(tofrom: a[0:n*m], b[0:n*m]) map(from: c[0:n*m]) \
                private(i, j) shared(a, b, c)
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            int idx = i * m + j;
            c[idx] = a[idx] + b[idx];
        }
    }
}

/* Alternative function with nested teams/distribute constructs */
void compute_nested_simt(int n, float *x, float *y, float *z) {
    /* Explicit nesting: target teams containing distribute parallel for simd */
    #pragma omp target teams map(tofrom: x[0:n], y[0:n]) map(from: z[0:n])
    {
        #pragma omp distribute parallel for simd \
                    private(i) shared(x, y, z)
        for (int i = 0; i < n; i++) {
            z[i] = x[i] * y[i];
        }
    }
}

/* Function with target simd that might trigger parallel transformation */
void compute_target_simd(int n, float *in, float *out, float scale) {
    volatile int use_gpu = 1; /* volatile to prevent constant folding */
    
    if (use_gpu) {
        /* target simd with parallel execution context */
        #pragma omp target simd map(to: in[0:n]) map(from: out[0:n]) \
                    private(i) linear(i:1)
        for (int i = 0; i < n; i++) {
            out[i] = in[i] * scale;
        }
    }
}

/* Complex loop structure with non-constant bounds */
void compute_variable_bounds(int start, int end, int step, 
                            float *src, float *dst, float coeff) {
    int i;
    /* Dynamic bounds from parameters to prevent constant folding */
    #pragma omp target teams distribute parallel for simd \
                map(to: src[start:end-start]) map(from: dst[start:end-start]) \
                private(i)
    for (i = start; i < end; i += step) {
        dst[i] = src[i] * coeff + i;
    }
}

/* Helper function for verification */
int verify_results(float *cpu, float *gpu, int size, float tolerance) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (fabs(cpu[i] - gpu[i]) > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at %d: CPU=%f, GPU=%f\n", i, cpu[i], gpu[i]);
            }
        }
    }
    return errors;
}

int main() {
    int i, j;
    
    /* Dynamic allocation with non-constant size */
    int size1 = N * M;
    int size2 = 2048;
    
    float *a = (float *)malloc(size1 * sizeof(float));
    float *b = (float *)malloc(size1 * sizeof(float));
    float *c_gpu = (float *)malloc(size1 * sizeof(float));
    float *c_cpu = (float *)malloc(size1 * sizeof(float));
    
    float *x = (float *)malloc(size2 * sizeof(float));
    float *y = (float *)malloc(size2 * sizeof(float));
    float *z_gpu = (float *)malloc(size2 * sizeof(float));
    float *z_cpu = (float *)malloc(size2 * sizeof(float));
    
    /* Initialize data */
    for (i = 0; i < size1; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c_cpu[i] = a[i] + b[i]; /* CPU reference */
    }
    
    for (i = 0; i < size2; i++) {
        x[i] = (float)i;
        y[i] = (float)(i + 1);
        z_cpu[i] = x[i] * y[i]; /* CPU reference */
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Primary SIMT transformation with collapse */
    printf("Test 1: teams distribute parallel for simd with collapse(2)\n");
    compute_simt(N, M, a, b, c_gpu);
    
    /* Verification */
    int errors1 = verify_results(c_cpu, c_gpu, size1, 1e-6f);
    printf("Test 1 errors: %d\n", errors1);
    
    /* Test 2: Nested teams/distribute construct */
    printf("\nTest 2: Nested teams with distribute parallel for simd\n");
    compute_nested_simt(size2, x, y, z_gpu);
    
    int errors2 = verify_results(z_cpu, z_gpu, size2, 1e-6f);
    printf("Test 2 errors: %d\n", errors2);
    
    /* Test 3: Multiple invocations with different contexts */
    printf("\nTest 3: Multiple invocations from different call sites\n");
    for (int iter = 0; iter < 3; iter++) {
        compute_simt(N >> iter, M >> iter, a, b, c_gpu);
        printf("  Iteration %d complete\n", iter);
    }
    
    /* Test 4: Variable bounds to prevent constant folding */
    printf("\nTest 4: Variable loop bounds\n");
    compute_variable_bounds(100, 500, 2, a, c_gpu, 2.5f);
    
    /* Test 5: Conditional execution path */
    printf("\nTest 5: Conditional target simd\n");
    float *in = (float *)malloc(1000 * sizeof(float));
    float *out_gpu = (float *)malloc(1000 * sizeof(float));
    float *out_cpu = (float *)malloc(1000 * sizeof(float));
    
    for (i = 0; i < 1000; i++) {
        in[i] = (float)i;
        out_cpu[i] = in[i] * 3.14f;
    }
    
    compute_target_simd(1000, in, out_gpu, 3.14f);
    
    int errors5 = verify_results(out_cpu, out_gpu, 1000, 1e-6f);
    printf("Test 5 errors: %d\n", errors5);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c_gpu);
    free(c_cpu);
    free(x);
    free(y);
    free(z_gpu);
    free(z_cpu);
    free(in);
    free(out_gpu);
    free(out_cpu);
    
    printf("\nAll tests completed.\n");
    
    return 0;
}
