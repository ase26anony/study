#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512

/* Function 1: Uses target teams distribute parallel for simd */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step) {
    volatile int vsize = end - start; /* volatile to prevent constant folding */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end], b[start:end]) map(from: c[start:end]) \
        firstprivate(vsize, step) shared(a, b, c)
    for (int i = start; i < end; i += step) {
        int idx = i;
        if (idx < end) {
            c[idx] = a[idx] + b[idx] * (vsize % 7);
        }
    }
}

/* Function 2: Uses target teams distribute parallel for (no simd clause) */
void parallel_target_loop(float *x, float *y, float scale, int low, int high) {
    volatile int vlow = low;
    volatile int vhigh = high;
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high]) map(tofrom: y[low:high]) \
        firstprivate(scale, vlow, vhigh)
    for (int i = vlow; i < vhigh; ++i) {
        y[i] = x[i] * scale + (float)(i % 5);
    }
}

/* Function 3: Combined constructs with target data and nested loops */
void combined_constructs(double *mat1, double *mat2, int rows, int cols, int stride) {
    static double local_sum = 0.0; /* static storage */
    const int chunk = 16; /* const qualifier */
    int i, j;
    
    #pragma omp target data map(to: mat1[0:rows*cols:stride]) \
                            map(tofrom: mat2[0:rows*cols:stride])
    {
        #pragma omp target teams distribute parallel for simd collapse(2) \
            private(i, j) firstprivate(rows, cols, chunk) reduction(+:local_sum)
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                int idx = i * cols + j;
                mat2[idx] = mat1[idx] * 2.5 + (double)(idx % 3);
                if (j % chunk == 0) {
                    local_sum += mat2[idx];
                }
            }
        }
    }
    /* Use local_sum to prevent dead code elimination */
    printf("  Local sum: %f\n", local_sum);
}

/* Function 4: Host-only parallel region (no target) */
void host_only_parallel(int *arr, int size, int factor) {
    #pragma omp parallel for simd
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * factor + i;
    }
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Arrays with different storage durations and types */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(N * sizeof(int));
    float *x = (float *)malloc(M * sizeof(float));
    float *y = (float *)malloc(M * sizeof(float));
    double *mat1 = (double *)malloc(N * M * sizeof(double));
    double *mat2 = (double *)malloc(N * M * sizeof(double));
    
    /* Initialize with random/sequential data */
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = 0;
    }
    for (int i = 0; i < M; ++i) {
        x[i] = (float)(rand() % 100) / 10.0f;
        y[i] = 0.0f;
    }
    for (int i = 0; i < N * M; ++i) {
        mat1[i] = (double)(rand() % 100) / 5.0;
        mat2[i] = 0.0;
    }
    
    /* Runtime-dependent parameters */
    volatile int use_target = (rand() % 3) > 0; /* volatile to prevent constant propagation */
    int iterations = 5;
    
    for (int iter = 0; iter < iterations; ++iter) {
        printf("Iteration %d:\n", iter);
        
        /* Vary loop bounds and array slices */
        int start = rand() % (N/4);
        int end = N - rand() % (N/4);
        int step = 1 + rand() % 3;
        int low = rand() % (M/2);
        int high = M - rand() % (M/4);
        float scale = 1.0f + (rand() % 10) / 10.0f;
        int rows = 64 + rand() % 64;
        int cols = 32 + rand() % 32;
        int stride = 1 + rand() % 2;
        
        /* Conditional execution to influence SIMT transformation */
        if (use_target || (iter % 2 == 0)) {
            printf("  Calling simd_target_loop\n");
            simd_target_loop(a, b, c, start, end, step);
            
            printf("  Calling parallel_target_loop\n");
            parallel_target_loop(x, y, scale, low, high);
            
            printf("  Calling combined_constructs\n");
            combined_constructs(mat1, mat2, rows, cols, stride);
        } else {
            printf("  Calling host_only_parallel\n");
            host_only_parallel(a, N, iter + 1);
        }
        
        /* Simple checksum to prevent dead code elimination */
        int sum_c = 0;
        for (int i = 0; i < N; ++i) sum_c += c[i];
        printf("  Checksum c: %d\n", sum_c);
        
        float sum_y = 0.0f;
        for (int i = 0; i < M; ++i) sum_y += y[i];
        printf("  Checksum y: %f\n", sum_y);
        
        double sum_mat2 = 0.0;
        for (int i = 0; i < rows * cols; ++i) sum_mat2 += mat2[i];
        printf("  Checksum mat2: %f\n", sum_mat2);
    }
    
    free(a); free(b); free(c);
    free(x); free(y);
    free(mat1); free(mat2);
    
    return 0;
}
