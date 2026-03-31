#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    int n, float alpha, int dynamic_count) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n], alpha) map(from: c[0:n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(32)
    for (int i = 0; i < n; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = alpha * a[i] + b[i];
        } else {
            c[i] = alpha * b[i] + a[i];
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_count], b[0:dynamic_count]) \
        map(tofrom: c[0:dynamic_count]) \
        device(0) num_teams(16) thread_limit(128) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* More complex conditional */
        if (i % 3 == 0) {
            c[i] = sqrtf(a[i] * a[i] + b[i] * b[i]);
        } else if (i % 3 == 1) {
            c[i] = a[i] * b[i] - c[i];
        } else {
            c[i] = (a[i] + b[i]) / 2.0f;
        }
    }
}

/* Host-side parallel function */
static void host_parallel_computation(float* a, float* b, float* c_ref, int n, float alpha) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + b[i];
        } else {
            c_ref[i] = alpha * b[i] + a[i];
        }
    }
}

int main() {
    float *a, *b, *c, *c_ref;
    int i;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c || !c_ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    #pragma omp parallel for
    for (i = 0; i < N; i++) {
        a[i] = (float)i / N;
        b[i] = (float)(N - i) / N;
        c[i] = 0.0f;
        c_ref[i] = 0.0f;
    }
    
    float alpha = 2.5f;
    int dynamic_count = M;  /* Dynamic iteration count for second loop */
    
    /* Call the GPU processing function from within a host parallel region */
    #pragma omp parallel
    {
        /* Each thread could potentially call the offloading function */
        #pragma omp single
        {
            process_on_gpu(a, b, c, N, alpha, dynamic_count);
        }
    }
    
    /* Compute reference on host */
    host_parallel_computation(a, b, c_ref, N, alpha);
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-5f;
    
    for (i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Mismatch at index %d: GPU=%f, Host=%f\n", 
                       i, c[i], c_ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All %d elements computed correctly on GPU\n", N);
    } else {
        printf("FAILURE: %d errors found\n", errors);
    }
    
    /* Clean up */
    free(a);
    free(b);
    free(c);
    free(c_ref);
    
    return errors > 0 ? 1 : 0;
}
