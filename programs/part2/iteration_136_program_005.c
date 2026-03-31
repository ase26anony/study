/* Test program to trigger SIMT transformation in GCC's omp-low.cc
 * Specifically targets lines 2941-2975 in the SIMT transformation pass
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000
#define VALIDATION_TOL 1e-6

/* Non-inlineable helper function containing target offloading regions */
static void __attribute__((noinline)) process_on_gpu(float* restrict a, 
                                                     float* restrict b, 
                                                     float* restrict c, 
                                                     int dynamic_count, 
                                                     float scale) {
    /* First target region: compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = a[i] * scale + b[i];
        } else {
            c[i] = a[i] * (scale * 0.5f) + b[i] * 2.0f;
        }
    }
    
    /* Second target region: dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_count], b[0:dynamic_count]) \
        map(tofrom: c[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* More complex conditional with nested arithmetic */
        if (i % 3 == 0) {
            c[i] = c[i] * 0.75f + a[i] * b[i];
        } else if (i % 3 == 1) {
            c[i] = sqrtf(fabsf(c[i])) + a[i] - b[i];
        } else {
            c[i] = (c[i] + a[i] + b[i]) / 3.0f;
        }
    }
    
    /* Third loop with reduction pattern */
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
        map(to: c[0:M]) map(tofrom: sum) reduction(+:sum) \
        device(0) num_teams(16) thread_limit(64) simdlen(8)
    for (int i = 0; i < M; i++) {
        /* Conditional reduction */
        if (c[i] > 0.0f) {
            sum += c[i];
        } else {
            sum -= c[i] * 0.5f;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    c[0] += sum * 0.001f;
}

/* Host-side reference computation for validation */
static void compute_reference(float* restrict a, float* restrict b,
                              float* restrict c_ref, int dynamic_count,
                              float scale) {
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            c_ref[i] = a[i] * scale + b[i];
        } else {
            c_ref[i] = a[i] * (scale * 0.5f) + b[i] * 2.0f;
        }
    }
    
    for (int i = 0; i < dynamic_count; i++) {
        if (i % 3 == 0) {
            c_ref[i] = c_ref[i] * 0.75f + a[i] * b[i];
        } else if (i % 3 == 1) {
            c_ref[i] = sqrtf(fabsf(c_ref[i])) + a[i] - b[i];
        } else {
            c_ref[i] = (c_ref[i] + a[i] + b[i]) / 3.0f;
        }
    }
    
    float sum = 0.0f;
    for (int i = 0; i < M; i++) {
        if (c_ref[i] > 0.0f) {
            sum += c_ref[i];
        } else {
            sum -= c_ref[i] * 0.5f;
        }
    }
    c_ref[0] += sum * 0.001f;
}

/* Wrapper function called from host-side OpenMP parallel region */
static void process_wrapper(float* a, float* b, float* c, 
                           float* c_ref, int dynamic_count) {
    float scale = 2.5f;
    
    /* Call the GPU processing function */
    process_on_gpu(a, b, c, dynamic_count, scale);
    
    /* Compute reference on host */
    compute_reference(a, b, c_ref, dynamic_count, scale);
}

int main() {
    float *a, *b, *c, *c_ref;
    int dynamic_count = 7500;  /* Dynamic value, not compile-time constant */
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c || !c_ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; i++) {
        a[i] = (i % 100) * 0.01f;
        b[i] = (i % 50) * 0.02f;
        c[i] = 0.0f;
        c_ref[i] = 0.0f;
    }
    
    /* Host-side OpenMP parallel region creating nested parallelism */
    #pragma omp parallel num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread processes a different slice */
        int slice_size = N / 4;
        int start = thread_id * slice_size;
        int end = (thread_id == 3) ? N : start + slice_size;
        
        /* Adjust dynamic count per thread to increase variation */
        int local_dynamic = dynamic_count - thread_id * 100;
        if (local_dynamic < 1000) local_dynamic = 1000;
        
        /* Call wrapper which contains target regions */
        process_wrapper(&a[start], &b[start], &c[start], 
                       &c_ref[start], local_dynamic);
    }
    
    /* Validate results */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > VALIDATION_TOL) {
            errors++;
            if (errors <= 10) {
                printf("Mismatch at index %d: device=%f, host=%f, diff=%e\n",
                       i, c[i], c_ref[i], fabsf(c[i] - c_ref[i]));
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All results match within tolerance %e\n", VALIDATION_TOL);
    } else {
        printf("FAILURE: %d mismatches found\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(c_ref);
    
    return (errors == 0) ? 0 : 1;
}
