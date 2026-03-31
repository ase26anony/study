#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define SUB_SIZE 256

/* Variant 1: SIMD target loop with collapse */
__attribute__((noinline))
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step) {
    volatile int vsize = end - start; /* volatile to prevent constant folding */
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: a[start:end], b[start:end]) map(from: c[start:end]) \
        firstprivate(start, end, step) shared(a, b, c)
    for (int i = start; i < end; i += step) {
        for (int j = 0; j < step && (i + j) < end; ++j) {
            int idx = i + j;
            /* Complex indexing to stress dependency analysis */
            c[idx] = a[idx] * 2 + b[idx] / 3 + (idx % 7);
        }
    }
}

/* Variant 2: Parallel target loop without explicit SIMD clause */
__attribute__((noinline))
void parallel_target_loop(float *x, float *y, float *z, int low, int high, int stride) {
    static const float scale = 2.5f; /* mixed storage duration */
    volatile int vhigh = high; /* volatile to preserve loop structure */
    
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high:stride], y[low:high:stride]) \
        map(from: z[low:high:stride]) \
        private(low, high) firstprivate(stride, scale)
    for (int i = low; i < vhigh; i += stride) {
        /* Pointer arithmetic */
        float *xp = x + i;
        float *yp = y + i;
        float *zp = z + i;
        *zp = *xp * scale + *yp / scale - (i % 5);
    }
}

/* Variant 3: Combined constructs with data region */
__attribute__((noinline))
void combined_constructs(double *p, double *q, double *r, int n, int offset) {
    const double alpha = 1.5;
    volatile int vn = n;
    
    #pragma omp target data map(to: p[offset:vn], q[offset:vn]) map(from: r[offset:vn])
    {
        /* Nested loops with conditional SIMD */
        #pragma omp target teams distribute parallel for simd \
            firstprivate(alpha, offset) shared(p, q, r)
        for (int i = offset; i < offset + vn; ++i) {
            r[i] = p[i] * alpha - q[i] / alpha + (i % 11);
        }
    }
}

/* Host-only parallel region for conditional execution */
__attribute__((noinline))
void host_only_parallel(int *arr, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; ++i) {
        arr[i] = arr[i] * 3 + i;
    }
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) seed = atoi(argv[1]);
    srand(seed);
    
    /* Arrays with different storage classes */
    static int array1[SIZE];
    int array2[SIZE];
    float farray1[SIZE];
    float farray2[SIZE];
    double darray1[SIZE];
    double darray2[SIZE];
    int results[SIZE];
    float fresults[SIZE];
    double dresults[SIZE];
    
    /* Initialize with random data */
    for (int i = 0; i < SIZE; ++i) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        farray1[i] = (float)(rand() % 100) / 3.0f;
        farray2[i] = (float)(rand() % 100) / 7.0f;
        darray1[i] = (double)(rand() % 100) / 5.0;
        darray2[i] = (double)(rand() % 100) / 9.0;
    }
    
    int num_iterations = 5;
    volatile int use_target = (seed % 3); /* runtime-dependent condition */
    
    for (int iter = 0; iter < num_iterations; ++iter) {
        /* Varying parameters to create different transformation contexts */
        int start = rand() % (SIZE - SUB_SIZE);
        int end = start + SUB_SIZE + (rand() % 64);
        int step = 1 + (rand() % 4);
        int stride = 1 + (rand() % 3);
        int offset = rand() % (SIZE / 2);
        int n = SUB_SIZE + (rand() % 128);
        
        /* Conditional execution to influence SIMT transformation */
        if (use_target == 0 || (rand() % 2)) {
            printf("Iteration %d: Using target regions\n", iter);
            simd_target_loop(array1, array2, results, start, end, step);
            parallel_target_loop(farray1, farray2, fresults, start, end, stride);
            combined_constructs(darray1, darray2, dresults, n, offset);
        } else {
            printf("Iteration %d: Using host-only parallel\n", iter);
            host_only_parallel(results, SUB_SIZE);
        }
        
        /* Compute checksums to prevent dead code elimination */
        long checksum = 0;
        float fchecksum = 0.0f;
        double dchecksum = 0.0;
        for (int i = 0; i < SUB_SIZE; ++i) {
            if (start + i < SIZE) {
                checksum += results[start + i];
                fchecksum += fresults[start + i];
                dchecksum += dresults[offset + i];
            }
        }
        printf("  Checksums: %ld, %.2f, %.2f\n", checksum, fchecksum, dchecksum);
        
        /* Modify parameters for next iteration */
        use_target = (use_target + iter) % 3;
    }
    
    return 0;
}
