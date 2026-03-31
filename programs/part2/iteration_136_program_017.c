/* Test program to trigger SIMT transformation in GCC's omp-low.cc
 * Specifically targets lines 2941-2975 in the SIMT loop transformation
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define N 10000
#define CHUNK_SIZE 128

/* Non-inlineable helper function containing target offloading regions */
static void __attribute__((noinline,noipa))
process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                int n, float scale, int dynamic_iterations)
{
    /* First target region: compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        device(0) num_teams(16) thread_limit(256) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = a[i] + scale * b[i];  /* SAXPY-like operation */
        } else {
            c[i] = a[i] - scale * b[i];  /* Alternative path */
        }
    }
    
    /* Second target region: dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_iterations], b[0:dynamic_iterations]) \
        map(from: c[0:dynamic_iterations]) \
        device(0) num_teams(8) thread_limit(128) simdlen(16)
    for (int i = 0; i < dynamic_iterations; i++) {
        /* More complex conditional with math function */
        if (i % 3 == 0) {
            c[i] = a[i] * sinf(scale * b[i]);
        } else if (i % 3 == 1) {
            c[i] = a[i] * cosf(scale * b[i]);
        } else {
            c[i] = a[i] * tanf(scale * b[i]);
        }
    }
    
    /* Third loop with reduction pattern */
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
        map(to: c[0:n]) map(tofrom: sum) reduction(+:sum) \
        device(0) num_teams(4) thread_limit(64) simdlen(8)
    for (int i = 0; i < n; i++) {
        /* Nested conditional with early continue */
        if (c[i] < 0.0f) {
            continue;
        }
        sum += c[i] * c[i];
    }
    
    /* Store the reduction result */
    c[0] = sum;
}

/* Wrapper function called from host OpenMP parallel region */
static void __attribute__((noinline))
gpu_processing_wrapper(float* a, float* b, float* c, int n, int dynamic_n)
{
    /* Host-side OpenMP parallel region creating nested parallelism */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        float scale = 1.0f + tid * 0.1f;
        
        /* Each thread calls the GPU processing function */
        process_on_gpu(a + tid * (n/2), b + tid * (n/2), 
                       c + tid * (n/2), n/2, scale, dynamic_n/2);
    }
}

/* Reference computation on host */
static void compute_reference(float* a, float* b, float* ref, int n, float scale)
{
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            ref[i] = a[i] + scale * b[i];
        } else {
            ref[i] = a[i] - scale * b[i];
        }
    }
}

int main(void)
{
    float *a, *b, *c, *ref;
    int dynamic_iterations = 5000;
    
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
        a[i] = (float)i / 100.0f;
        b[i] = (float)(i % 100) / 50.0f;
        c[i] = 0.0f;
        ref[i] = 0.0f;
    }
    
    /* Compute reference on host */
    compute_reference(a, b, ref, N, 1.5f);
    
    /* Reset device results */
    for (int i = 0; i < N; i++) {
        c[i] = 0.0f;
    }
    
    printf("Starting GPU offloading with SIMT transformation test...\n");
    
    /* Call the wrapper which contains nested OpenMP */
    gpu_processing_wrapper(a, b, c, N, dynamic_iterations);
    
    /* Validate results for the first N/2 elements (processed by thread 0) */
    int errors = 0;
    float tolerance = 1e-4f;
    
    for (int i = 0; i < N/2; i++) {
        if (fabsf(c[i] - ref[i]) > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at index %d: device=%f, host=%f\n", 
                       i, c[i], ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All GPU computations match host reference\n");
        printf("SIMT transformation likely triggered in omp-low.cc\n");
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
