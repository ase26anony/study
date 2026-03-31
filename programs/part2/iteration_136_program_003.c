/* Test program to trigger SIMT transformation in GCC's omp-low.cc
 * Specifically targets lines 2941-2975 in the SIMT transformation pass
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function containing target offloading regions */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float scalar, int dynamic_count) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = a[i] * scalar + b[i];
        } else {
            c[i] = a[i] * (scalar * 0.5f) + b[i] * 2.0f;
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_count], b[0:dynamic_count]) \
        map(tofrom: c[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* More complex conditional to stress SIMT transformation */
        if (i % 3 == 0) {
            c[i] = sqrtf(fabsf(a[i])) + b[i];
        } else if (i % 3 == 1) {
            c[i] = a[i] * a[i] - b[i];
        } else {
            c[i] = (a[i] + b[i]) * scalar;
        }
    }
}

/* Wrapper function called from host-side OpenMP parallel region */
static void gpu_wrapper(float* a, float* b, float* c, float scalar, int dynamic_count) {
    /* Additional host-side computation to create nesting context */
    #pragma omp for simd
    for (int i = 0; i < M; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(N - i) * 0.05f;
    }
    
    /* Call the function with target offloading */
    process_on_gpu(a, b, c, scalar, dynamic_count);
    
    /* More host-side computation */
    #pragma omp simd
    for (int i = 0; i < 100; i++) {
        c[i] += 1.0f;
    }
}

/* Reference computation for validation */
static void compute_reference(float* a, float* b, float* ref, float scalar, int n) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            ref[i] = a[i] * scalar + b[i];
        } else {
            ref[i] = a[i] * (scalar * 0.5f) + b[i] * 2.0f;
        }
    }
    
    for (int i = 0; i < n/2; i++) {
        if (i % 3 == 0) {
            ref[i] = sqrtf(fabsf(a[i])) + b[i];
        } else if (i % 3 == 1) {
            ref[i] = a[i] * a[i] - b[i];
        } else {
            ref[i] = (a[i] + b[i]) * scalar;
        }
    }
}

int main() {
    float *a, *b, *c, *ref;
    float scalar = 2.5f;
    int dynamic_count = N/2;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    ref = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c || !ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 0.01f;
        b[i] = (float)(N - i) * 0.02f;
        c[i] = 0.0f;
        ref[i] = 0.0f;
    }
    
    /* Host-side OpenMP parallel region wrapping target offloading */
    #pragma omp parallel num_threads(4)
    {
        gpu_wrapper(a, b, c, scalar, dynamic_count);
    }
    
    /* Compute reference on host */
    compute_reference(a, b, ref, scalar, dynamic_count);
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    for (int i = 0; i < dynamic_count; i++) {
        if (fabsf(c[i] - ref[i]) > tolerance) {
            errors++;
            if (errors <= 10) {
                printf("Mismatch at index %d: device=%f, host=%f\n", 
                       i, c[i], ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All %d elements match within tolerance %f\n", 
               dynamic_count, tolerance);
    } else {
        printf("FAILURE: %d mismatches out of %d elements\n", 
               errors, dynamic_count);
    }
    
    /* Additional test with different parameters */
    printf("\nRunning second test with different parameters...\n");
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp target teams distribute parallel for simd \
                map(to: a[0:1000]) map(tofrom: b[0:1000]) \
                device(0) num_teams(16) thread_limit(64) simdlen(8)
            for (int i = 0; i < 1000; i++) {
                /* Nested conditional to increase transformation complexity */
                if (i < 500) {
                    b[i] = a[i] * (i % 10);
                } else {
                    if (a[i] > 0) {
                        b[i] = logf(a[i] + 1.0f);
                    } else {
                        b[i] = -logf(-a[i] + 1.0f);
                    }
                }
            }
        }
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(ref);
    
    return errors > 0 ? 1 : 0;
}
