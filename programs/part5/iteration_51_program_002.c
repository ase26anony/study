/* Test program to trigger SIMT transformation in GCC omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    /* This construct should trigger IFN_GOMP_USE_SIMT generation */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: result[0:N]) \
        private(i) shared(result) num_teams(8) thread_limit(256)
    for (int i = 0; i < N; i++) {
        result[i] = a[i] * b[i] + a[i] / (b[i] + 1.0f);
    }
}

/* Alternative function with nested constructs */
void compute_nested_simt(int M, int N, float *matrix, float *vector, float *output) {
    int i, j;
    
    /* Explicit nesting that should also trigger SIMT transformation */
    #pragma omp target teams map(to: matrix[0:M*N], vector[0:N]) map(from: output[0:M])
    {
        #pragma omp distribute
        for (i = 0; i < M; i++) {
            #pragma omp parallel for simd reduction(+:output[i]) private(j)
            for (j = 0; j < N; j++) {
                output[i] += matrix[i * N + j] * vector[j];
            }
        }
    }
}

/* Function with collapse clause to increase complexity */
void compute_collapsed_simt(int N, int M, float *a, float *b, float *c) {
    /* Using collapse to create more complex loop structure */
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: a[0:N*M], b[0:N*M]) map(from: c[0:N*M]) \
        private(i,j) shared(c)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c[idx] = a[idx] * 2.0f + b[idx] * 3.0f;
        }
    }
}

/* Function with conditional execution path */
void conditional_simt(int N, float *data, float threshold, int use_simd) {
    volatile int dynamic_flag = use_simd; /* volatile to prevent constant folding */
    
    if (dynamic_flag) {
        /* This should create the conditional branching seen in uncovered code */
        #pragma omp target simd map(tofrom: data[0:N]) if(target: dynamic_flag)
        for (int i = 0; i < N; i++) {
            if (data[i] > threshold) {
                data[i] = data[i] * 2.0f;
            } else {
                data[i] = data[i] / 2.0f;
            }
        }
    }
}

/* Helper function for verification */
int verify_results(float *cpu, float *gpu, int N, float tolerance) {
    for (int i = 0; i < N; i++) {
        if (fabs(cpu[i] - gpu[i]) > tolerance) {
            printf("Mismatch at index %d: CPU=%f, GPU=%f\n", i, cpu[i], gpu[i]);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent loop elimination */
    int N = (argc > 1) ? atoi(argv[1]) : 1024;
    int M = (argc > 2) ? atoi(argv[2]) : 512;
    
    /* Dynamic allocation to avoid constant propagation */
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *result_gpu = (float *)malloc(N * sizeof(float));
    float *result_cpu = (float *)malloc(N * sizeof(float));
    
    float *matrix = (float *)malloc(M * N * sizeof(float));
    float *vector = (float *)malloc(N * sizeof(float));
    float *output_gpu = (float *)malloc(M * sizeof(float));
    float *output_cpu = (float *)malloc(M * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 1) % 100) * 0.2f;
        vector[i] = (float)(i % 50) * 0.05f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i * N + j] = (float)((i + j) % 100) * 0.01f;
        }
    }
    
    /* Test 1: Basic SIMT transformation */
    printf("Test 1: Basic target teams distribute parallel for simd\n");
    compute_simt(N, a, b, result_gpu);
    
    /* CPU verification */
    for (int i = 0; i < N; i++) {
        result_cpu[i] = a[i] * b[i] + a[i] / (b[i] + 1.0f);
    }
    
    if (verify_results(result_cpu, result_gpu, N, 1e-5f)) {
        printf("Test 1 passed\n");
    }
    
    /* Test 2: Nested construct */
    printf("\nTest 2: Nested teams/distribute/parallel for simd\n");
    compute_nested_simt(M, N, matrix, vector, output_gpu);
    
    /* CPU verification for matrix-vector multiplication */
    for (int i = 0; i < M; i++) {
        output_cpu[i] = 0.0f;
        for (int j = 0; j < N; j++) {
            output_cpu[i] += matrix[i * N + j] * vector[j];
        }
    }
    
    if (verify_results(output_cpu, output_gpu, M, 1e-5f)) {
        printf("Test 2 passed\n");
    }
    
    /* Test 3: Collapsed loops */
    printf("\nTest 3: Collapsed SIMT loops\n");
    float *c_gpu = (float *)malloc(N * M * sizeof(float));
    float *c_cpu = (float *)malloc(N * M * sizeof(float));
    
    compute_collapsed_simt(N, M, a, b, c_gpu);
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c_cpu[idx] = a[i] * 2.0f + b[i] * 3.0f;
        }
    }
    
    if (verify_results(c_cpu, c_gpu, N * M, 1e-5f)) {
        printf("Test 3 passed\n");
    }
    
    /* Test 4: Conditional SIMT execution */
    printf("\nTest 4: Conditional SIMT execution\n");
    float *data = (float *)malloc(N * sizeof(float));
    for (int i = 0; i < N; i++) {
        data[i] = (float)i * 0.1f;
    }
    
    /* Call with different conditions to explore both paths */
    conditional_simt(N, data, 5.0f, 1);  /* Should use SIMT */
    conditional_simt(N, data, 5.0f, 0);  /* Should not use SIMT */
    
    printf("Test 4 completed\n");
    
    /* Test 5: Multiple calls from different contexts */
    printf("\nTest 5: Multiple invocations from different call sites\n");
    for (int iter = 0; iter < 3; iter++) {
        if (iter % 2 == 0) {
            compute_simt(N / (iter + 1), a, b, result_gpu);
        } else {
            compute_nested_simt(M / (iter + 1), N / (iter + 1), matrix, vector, output_gpu);
        }
    }
    printf("Test 5 completed\n");
    
    /* Cleanup */
    free(a); free(b); free(result_gpu); free(result_cpu);
    free(matrix); free(vector); free(output_gpu); free(output_cpu);
    free(c_gpu); free(c_cpu); free(data);
    
    return 0;
}
