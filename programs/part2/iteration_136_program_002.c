#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define VALIDATION_TOL 1e-6

/* Non-inlineable helper function containing target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* a, float* b, float* c, int n, float scale) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n], scale) map(from: c[0:n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = a[i] * scale + b[i];
        } else {
            c[i] = a[i] * (scale * 0.5f) + b[i] * 2.0f;
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: c[0:n]) map(to: scale) \
        device(0) num_teams(16) thread_limit(128) simdlen(16)
    for (int i = 0; i < n; i++) {
        /* Another conditional with different pattern */
        if (i % 3 == 0) {
            c[i] = sqrtf(fabsf(c[i])) * scale;
        } else if (i % 3 == 1) {
            c[i] = c[i] * c[i] * scale;
        } else {
            c[i] = c[i] + scale;
        }
    }
}

/* Host-side reference computation for validation */
static void compute_reference(float* a, float* b, float* ref, int n, float scale) {
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            ref[i] = a[i] * scale + b[i];
        } else {
            ref[i] = a[i] * (scale * 0.5f) + b[i] * 2.0f;
        }
    }
    
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            ref[i] = sqrtf(fabsf(ref[i])) * scale;
        } else if (i % 3 == 1) {
            ref[i] = ref[i] * ref[i] * scale;
        } else {
            ref[i] = ref[i] + scale;
        }
    }
}

int main() {
    float *a, *b, *c, *ref;
    int i;
    float scale = 2.5f;
    int dynamic_n = 5000;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    ref = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c || !ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with some pattern */
    #pragma omp parallel for
    for (i = 0; i < N; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(N - i) * 0.05f;
        c[i] = 0.0f;
        ref[i] = 0.0f;
    }
    
    /* Host-side OpenMP parallel region wrapping the GPU call */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        if (tid == 0) {
            /* Call the function containing target offloading */
            process_on_gpu(a, b, c, dynamic_n, scale);
        }
    }
    
    /* Compute reference on host */
    compute_reference(a, b, ref, dynamic_n, scale);
    
    /* Validate results */
    int errors = 0;
    for (i = 0; i < dynamic_n; i++) {
        if (fabsf(c[i] - ref[i]) > VALIDATION_TOL) {
            errors++;
            if (errors <= 10) {
                printf("Mismatch at index %d: device=%f, host=%f\n", 
                       i, c[i], ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All %d elements match within tolerance %e\n", 
               dynamic_n, VALIDATION_TOL);
    } else {
        printf("FAILURE: %d/%d elements mismatched\n", errors, dynamic_n);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(ref);
    
    return errors > 0 ? 1 : 0;
}
