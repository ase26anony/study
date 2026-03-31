/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function containing target offloading regions */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    int n, float scale, int dynamic_count) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < n; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = a[i] * scale + b[i];
        } else {
            c[i] = a[i] * (scale * 0.5f) + b[i] * 2.0f;
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_count]) map(tofrom: b[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* More complex conditional with math function */
        if (i % 3 == 0) {
            b[i] = c[i] * sinf(a[i] * 0.01f);
        } else if (i % 3 == 1) {
            b[i] = c[i] * cosf(a[i] * 0.01f);
        } else {
            b[i] = sqrtf(fabsf(c[i])) + a[i];
        }
    }
    
    /* Third loop with reduction - stresses transformation differently */
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
        map(to: c[0:n]) reduction(+:sum) \
        device(0) num_teams(16) thread_limit(64) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += c[i] * (i % 10);
    }
    
    /* Use the result to prevent dead code elimination */
    c[0] += sum * 0.001f;
}

/* Wrapper function called from host-side OpenMP parallel region */
static void gpu_processing_wrapper(float* a, float* b, float* c, 
                                   int n, int dynamic_count) {
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        float scale = 1.0f + tid * 0.5f;
        
        /* Each thread calls the offloading function with different parameters */
        process_on_gpu(a + tid * (n/2), b + tid * (n/2), c + tid * (n/2),
                      n/2, scale, dynamic_count/2);
    }
}

/* Host-side reference computation for validation */
static void host_reference(float* a, float* b, float* c_ref, 
                          int n, float scale) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = a[i] * scale + b[i];
        } else {
            c_ref[i] = a[i] * (scale * 0.5f) + b[i] * 2.0f;
        }
    }
}

int main() {
    float *a, *b, *c, *c_ref;
    int dynamic_count = M;
    
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
        a[i] = (i % 100) * 0.1f;
        b[i] = (i % 50) * 0.2f;
        c[i] = 0.0f;
        c_ref[i] = 0.0f;
    }
    
    /* Compute reference on host */
    host_reference(a, b, c_ref, N, 1.5f);
    
    /* Reset device arrays */
    for (int i = 0; i < N; i++) {
        c[i] = 0.0f;
    }
    
    /* Execute GPU offloading with nested parallelism */
    gpu_processing_wrapper(a, b, c, N, dynamic_count);
    
    /* Validate results - allow small differences for floating point */
    int errors = 0;
    float tolerance = 1e-4f;
    for (int i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
            if (errors <= 10) {
                printf("Mismatch at index %d: device=%f, host=%f\n", 
                       i, c[i], c_ref[i]);
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
    free(c_ref);
    
    return errors > 0 ? 1 : 0;
}
