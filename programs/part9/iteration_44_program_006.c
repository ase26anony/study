#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512

/* Variant 1: SIMD target loop with collapse */
static void simd_target_loop(int *a, int *b, int *c, int start, int end, int step) {
    volatile int vs = start;  /* Prevent constant folding */
    volatile int ve = end;
    volatile int vst = step;
    
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: a[vs:ve], b[vs:ve]) map(from: c[vs:ve]) \
        private(vs, ve, vst) shared(a, b, c)
    for (int i = vs; i < ve; i += vst) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            if (idx < N * M) {
                c[idx] = a[idx] + b[idx] * (i % 8);  /* Complex indexing */
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
static void parallel_target_loop(float *x, float *y, float alpha, 
                                 int low, int high, int stride) {
    const float beta = 2.0f;
    static float gamma = 1.5f;  /* Mixed storage durations */
    
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high:stride], alpha) \
        map(tofrom: y[low:high:stride]) \
        firstprivate(beta) shared(gamma)
    for (int i = low; i < high; i += stride) {
        y[i] = alpha * x[i] + beta * y[i] + gamma;
        gamma = gamma * 0.99f;  /* Has side effect */
    }
}

/* Variant 3: Combined constructs with data region */
static void combined_constructs(double *mat, double *vec, double *res,
                                int rows, int cols, int offset) {
    double local_sum = 0.0;
    int *offsets = (int*)malloc(rows * sizeof(int));
    
    /* Initialize offsets with pointer arithmetic */
    for (int i = 0; i < rows; i++) {
        offsets[i] = offset + i * cols;
    }
    
    #pragma omp target data map(to: mat[0:rows*cols], vec[0:cols], offsets[0:rows]) \
                            map(tofrom: res[0:rows], local_sum)
    {
        #pragma omp target teams distribute parallel for simd \
            reduction(+:local_sum) \
            private(rows, cols) shared(mat, vec, res, offsets)
        for (int i = 0; i < rows; i++) {
            double sum = 0.0;
            #pragma omp simd reduction(+:sum)
            for (int j = 0; j < cols; j++) {
                int idx = offsets[i] + j;
                sum += mat[idx] * vec[j];
            }
            res[i] = sum;
            local_sum += sum;
        }
    }
    
    free(offsets);
}

/* Host-only parallel region for conditional execution */
static void host_only_parallel(int *arr, int size, int factor) {
    #pragma omp parallel for simd
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * factor + omp_get_thread_num();
    }
}

int main(int argc, char *argv[]) {
    /* Initialize with random seed from command line */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Declare arrays with different storage classes */
    static int array1[N * M];
    int array2[N * M];
    const int init_val = 42;
    volatile int v_size = N * M;  /* Volatile size */
    
    float farray1[N], farray2[N];
    double dmat[N * M], dvec[M], dres[N];
    
    /* Initialize with random data */
    for (int i = 0; i < N * M; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
    }
    
    for (int i = 0; i < N; i++) {
        farray1[i] = (float)rand() / RAND_MAX;
        farray2[i] = (float)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < N * M; i++) {
        dmat[i] = (double)rand() / RAND_MAX;
    }
    for (int i = 0; i < M; i++) {
        dvec[i] = (double)rand() / RAND_MAX;
    }
    
    int checksum = 0;
    
    /* Loop with varying parameters to expose different contexts */
    for (int iter = 0; iter < 5; iter++) {
        int use_target = (rand() % 3) > 0;  /* Randomly choose target vs host */
        int low = rand() % 100;
        int high = N - (rand() % 100);
        int stride = 1 + (rand() % 3);
        float alpha = (float)rand() / RAND_MAX * 2.0f;
        
        if (use_target) {
            printf("Iteration %d: Using target regions\n", iter);
            
            /* Call SIMD target variant */
            simd_target_loop(array1, array2, array2, 
                            low, high, stride);
            
            /* Call parallel target variant */
            parallel_target_loop(farray1, farray2, alpha,
                                low, high, stride);
            
            /* Call combined constructs variant */
            int rows = N / (iter + 1);
            int cols = M / (iter + 1);
            combined_constructs(dmat, dvec, dres, rows, cols, iter * 10);
        } else {
            printf("Iteration %d: Using host-only parallel\n", iter);
            host_only_parallel(array1, v_size, iter + 1);
        }
        
        /* Compute checksum to prevent dead code elimination */
        for (int i = 0; i < 100; i++) {
            int idx = rand() % (N * M);
            checksum += array1[idx] + array2[idx];
        }
        
        /* Verify results */
        printf("  Checksum increment: %d\n", checksum);
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
