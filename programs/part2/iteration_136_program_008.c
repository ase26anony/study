/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000
#define VALIDATE_TOL 1e-6

/* Non-inlineable helper function containing target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    int n, float alpha, int dynamic_count) {
    /* First target region: compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < n; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = alpha * a[i] + b[i];
        } else {
            c[i] = alpha * b[i] + a[i];
        }
    }
    
    /* Second target region: dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_count]) map(tofrom: c[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* More complex conditional */
        if (i % 3 == 0) {
            c[i] = sqrtf(fabsf(c[i])) + a[i];
        } else if (i % 3 == 1) {
            c[i] = c[i] * c[i] - b[i];
        } else {
            c[i] = c[i] + a[i] * b[i];
        }
    }
}

/* Host-side reference computation */
static void compute_reference(float* a, float* b, float* ref, int n, 
                              float alpha, int dynamic_count) {
    /* First loop reference */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            ref[i] = alpha * a[i] + b[i];
        } else {
            ref[i] = alpha * b[i] + a[i];
        }
    }
    
    /* Second loop reference */
    for (int i = 0; i < dynamic_count; i++) {
        if (i % 3 == 0) {
            ref[i] = sqrtf(fabsf(ref[i])) + a[i];
        } else if (i % 3 == 1) {
            ref[i] = ref[i] * ref[i] - b[i];
        } else {
            ref[i] = ref[i] + a[i] * b[i];
        }
    }
}

/* Wrapper called from host OpenMP parallel region */
static void gpu_wrapper(float* a, float* b, float* c, int n, 
                        float alpha, int dynamic_count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        /* Each thread calls the GPU offloading function */
        if (tid == 0) {
            process_on_gpu(a, b, c, n, alpha, dynamic_count);
        }
        #pragma omp barrier
    }
}

int main() {
    float *a, *b, *c, *ref;
    float alpha = 2.5f;
    int dynamic_count = M;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    ref = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c || !ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 50) % 100) * 0.2f;
        c[i] = 0.0f;
        ref[i] = 0.0f;
    }
    
    /* Compute reference on host */
    compute_reference(a, b, ref, N, alpha, dynamic_count);
    
    /* Reset device array */
    for (int i = 0; i < N; i++) {
        c[i] = 0.0f;
    }
    
    /* Call GPU processing from host OpenMP parallel region */
    #pragma omp parallel num_threads(4)
    {
        gpu_wrapper(a, b, c, N, alpha, dynamic_count);
    }
    
    /* Validate results */
    int errors = 0;
    for (int i = 0; i < dynamic_count; i++) {
        if (fabsf(c[i] - ref[i]) > VALIDATE_TOL) {
            errors++;
            if (errors <= 10) {
                printf("Mismatch at index %d: device=%f, host=%f\n", 
                       i, c[i], ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All results match within tolerance\n");
    } else {
        printf("FAILURE: %d mismatches found\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(ref);
    
    return errors > 0 ? 1 : 0;
}
