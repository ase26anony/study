/* Test program to trigger SIMT transformation with IFN_GOMP_USE_SIMT */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    /* Primary target region designed for SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(result) \
        num_teams(128) thread_limit(64)
    for (int i = 0; i < dynamic_N; i++) {
        result[i] = a[i] * b[i] + (float)i / N;
    }
}

/* Alternative function with nested constructs */
void compute_nested_simt(int M, int N, float *matrix, float *vector, float *output) {
    volatile int rows = M;
    volatile int cols = N;
    
    /* Nested teams and distribute parallel for simd */
    #pragma omp target teams map(to: matrix[0:rows*cols], vector[0:cols]) \
                             map(from: output[0:rows]) \
                             num_teams(256)
    {
        #pragma omp distribute parallel for simd \
            private(i, j, sum) collapse(2) \
            reduction(+:sum)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                float sum = 0.0f;
                sum += matrix[i * cols + j] * vector[j];
                output[i] = sum;
            }
        }
    }
}

/* Function with target simd and parallel clause */
void compute_target_simd(int N, float *x, float *y, float alpha) {
    volatile int size = N;
    
    /* target simd with explicit parallel execution */
    #pragma omp target simd map(to: x[0:size]) map(tofrom: y[0:size]) \
        linear(i:1) aligned(x, y: 32) if(target: size > 1000)
    for (int i = 0; i < size; i++) {
        y[i] = alpha * x[i] + y[i];
    }
}

/* Helper function to verify results */
int verify_results(int N, float *cpu_result, float *gpu_result, float tolerance) {
    for (int i = 0; i < N; i++) {
        if (fabs(cpu_result[i] - gpu_result[i]) > tolerance) {
            fprintf(stderr, "Mismatch at index %d: CPU=%f, GPU=%f\n", 
                    i, cpu_result[i], gpu_result[i]);
            return 0;
        }
    }
    return 1;
}

/* CPU reference implementation */
void cpu_compute(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        result[i] = a[i] * b[i] + (float)i / N;
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent compile-time optimization */
    int base_size = 10000;
    if (argc > 1) base_size = atoi(argv[1]);
    
    /* Dynamic allocation prevents constant propagation */
    int N1 = base_size;
    int N2 = base_size * 2;
    int M = 512, N = 1024;
    
    /* Allocate arrays */
    float *a1 = (float *)malloc(N1 * sizeof(float));
    float *b1 = (float *)malloc(N1 * sizeof(float));
    float *result_gpu1 = (float *)malloc(N1 * sizeof(float));
    float *result_cpu1 = (float *)malloc(N1 * sizeof(float));
    
    float *a2 = (float *)malloc(N2 * sizeof(float));
    float *b2 = (float *)malloc(N2 * sizeof(float));
    float *result_gpu2 = (float *)malloc(N2 * sizeof(float));
    
    float *matrix = (float *)malloc(M * N * sizeof(float));
    float *vector = (float *)malloc(N * sizeof(float));
    float *output_gpu = (float *)malloc(M * sizeof(float));
    
    /* Initialize data */
    #pragma omp parallel for simd
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)i / N1;
        b1[i] = (float)(N1 - i) / N1;
    }
    
    #pragma omp parallel for simd
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)i / N2;
        b2[i] = (float)(N2 - i) / N2;
    }
    
    #pragma omp parallel for simd collapse(2)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i * N + j] = (float)(i + j) / (M + N);
        }
    }
    
    #pragma omp parallel for simd
    for (int j = 0; j < N; j++) {
        vector[j] = (float)j / N;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Primary SIMT transformation */
    printf("Test 1: Basic target teams distribute parallel for simd\n");
    compute_simt(N1, a1, b1, result_gpu1);
    cpu_compute(N1, a1, b1, result_cpu1);
    if (verify_results(N1, result_cpu1, result_gpu1, 1e-5f)) {
        printf("  Test 1 passed\n");
    }
    
    /* Test 2: Different size to trigger different contexts */
    printf("\nTest 2: Different data size\n");
    compute_simt(N2, a2, b2, result_gpu2);
    
    /* Test 3: Nested construct */
    printf("\nTest 3: Nested teams with distribute parallel for simd\n");
    compute_nested_simt(M, N, matrix, vector, output_gpu);
    
    /* Test 4: Conditional execution to affect current_function_decl context */
    printf("\nTest 4: Conditional target region\n");
    if (N1 > 5000) {
        compute_target_simd(N1, a1, b1, 2.0f);
    } else {
        compute_target_simd(N2, a2, b2, 0.5f);
    }
    
    /* Test 5: Inline region with collapse clause */
    printf("\nTest 5: Collapsed loops\n");
    volatile int dim1 = 256;
    volatile int dim2 = 256;
    float *c = (float *)malloc(dim1 * dim2 * sizeof(float));
    float *d = (float *)malloc(dim1 * dim2 * sizeof(float));
    
    #pragma omp target teams distribute parallel for simd \
        map(to: c[0:dim1*dim2]) map(from: d[0:dim1*dim2]) \
        collapse(2)
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            int idx = i * dim2 + j;
            d[idx] = c[idx] * 2.0f + (float)(i + j);
        }
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result_gpu1); free(result_cpu1);
    free(a2); free(b2); free(result_gpu2);
    free(matrix); free(vector); free(output_gpu);
    free(c); free(d);
    
    printf("\nAll SIMT tests completed\n");
    return 0;
}
