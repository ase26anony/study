/* Test program to trigger SIMT transformation in GCC's OpenMP offloading */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024
#define M 512

/* Function containing the primary SIMT transformation target */
void compute_simt(int n, int m, float *a, float *b, float *c) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int use_simt = 1;
    
    if (use_simt) {
        /* Primary target: teams distribute parallel for simd with collapse */
        #pragma omp target teams distribute parallel for simd \
            collapse(2) map(tofrom: a[0:n*m], b[0:n*m]) map(from: c[0:n*m]) \
            private(i, j) shared(a, b, c)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] + b[idx] * 2.0f;
            }
        }
    }
}

/* Alternative function with nested teams/distribute */
void compute_nested_simt(int n, float *x, float *y, float *z) {
    /* Non-constant bounds from parameters */
    int rows = n;
    int cols = n / 2;
    
    /* Nested teams and distribute parallel for simd */
    #pragma omp target teams map(tofrom: x[0:rows*cols], y[0:rows*cols]) \
        map(from: z[0:rows*cols])
    {
        #pragma omp distribute parallel for simd \
            private(i, j) shared(x, y, z) collapse(2)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                z[idx] = x[idx] * y[idx] + (i + j) * 0.5f;
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int n, float *in, float *out) {
    int chunk = n / 4;
    
    /* target simd in a context that suggests parallel execution */
    #pragma omp target simd map(to: in[0:n]) map(from: out[0:n]) \
        private(i) linear(i:1)
    for (int i = 0; i < n; i++) {
        out[i] = in[i] * in[i] + 1.0f;
    }
    
    /* Follow with a parallel region to create interesting context */
    #pragma omp parallel for simd
    for (int i = 0; i < chunk; i++) {
        out[i] += 0.1f;
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

/* CPU reference computation */
void cpu_compute(int n, int m, float *a, float *b, float *c) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            c[idx] = a[idx] + b[idx] * 2.0f;
        }
    }
}

int main() {
    /* Dynamic sizes to prevent constant propagation */
    int size1 = N * M;
    int size2 = N * N / 2;
    int size3 = N * 2;
    
    /* Allocate and initialize arrays */
    float *a1 = (float *)malloc(size1 * sizeof(float));
    float *b1 = (float *)malloc(size1 * sizeof(float));
    float *c1_gpu = (float *)malloc(size1 * sizeof(float));
    float *c1_cpu = (float *)malloc(size1 * sizeof(float));
    
    float *x2 = (float *)malloc(size2 * sizeof(float));
    float *y2 = (float *)malloc(size2 * sizeof(float));
    float *z2_gpu = (float *)malloc(size2 * sizeof(float));
    
    float *in3 = (float *)malloc(size3 * sizeof(float));
    float *out3_gpu = (float *)malloc(size3 * sizeof(float));
    
    /* Initialize with pattern data */
    for (int i = 0; i < size1; i++) {
        a1[i] = i * 0.1f;
        b1[i] = i * 0.2f;
    }
    
    for (int i = 0; i < size2; i++) {
        x2[i] = (i % 100) * 0.05f;
        y2[i] = (i % 50) * 0.1f;
    }
    
    for (int i = 0; i < size3; i++) {
        in3[i] = i * 0.25f;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Primary SIMT transformation */
    printf("Test 1: target teams distribute parallel for simd\n");
    compute_simt(N, M, a1, b1, c1_gpu);
    
    /* CPU reference */
    cpu_compute(N, M, a1, b1, c1_cpu);
    int errors1 = verify_results(c1_cpu, c1_gpu, size1, 0.001f);
    
    /* Test 2: Nested teams/distribute (different call context) */
    printf("Test 2: nested teams with distribute parallel for simd\n");
    compute_nested_simt(N, x2, y2, z2_gpu);
    
    /* Test 3: target simd with parallel context */
    printf("Test 3: target simd with parallel execution context\n");
    compute_target_simd(size3, in3, out3_gpu);
    
    /* Additional invocation from different control flow path */
    if (errors1 == 0) {
        printf("Re-running with success path...\n");
        compute_simt(N/2, M, a1, b1, c1_gpu);
    } else {
        printf("Re-running with error path...\n");
        compute_simt(N, M/2, a1, b1, c1_gpu);
    }
    
    /* Dead code with alternative construct to expose to parser */
    if (0) {
        /* This won't execute but will be seen by compiler */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: a1[0:size1])
        for (int i = 0; i < size1; i++) {
            a1[i] *= 2.0f;
        }
    }
    
    /* Cleanup */
    free(a1); free(b1); free(c1_gpu); free(c1_cpu);
    free(x2); free(y2); free(z2_gpu);
    free(in3); free(out3_gpu);
    
    printf("Tests completed. Errors in test 1: %d\n", errors1);
    
    return errors1 > 0 ? 1 : 0;
}
