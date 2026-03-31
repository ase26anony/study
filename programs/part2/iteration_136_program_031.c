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
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: c[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* Another conditional with different pattern */
        if (i % 3 == 0) {
            c[i] = sqrtf(c[i]);
        } else if (i % 3 == 1) {
            c[i] = c[i] * c[i];
        } else {
            c[i] = 1.0f / (c[i] + 1.0f);
        }
    }
}

/* Host-side reference computation for validation */
static void compute_reference(float* a, float* b, float* ref, int n, float alpha) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            ref[i] = alpha * a[i] + b[i];
        } else {
            ref[i] = alpha * b[i] + a[i];
        }
    }
}

int main(int argc, char** argv) {
    float *a, *b, *c, *ref;
    int i;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    ref = (float*)malloc(N * sizeof(float));
    
    for (i = 0; i < N; i++) {
        a[i] = (float)i / N;
        b[i] = (float)(N - i) / N;
        c[i] = 0.0f;
        ref[i] = 0.0f;
    }
    
    float alpha = 2.5f;
    int dynamic_count = (argc > 1) ? atoi(argv[1]) : M;
    if (dynamic_count > N) dynamic_count = N;
    
    /* Host-side OpenMP parallel region wrapping the offload call */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        if (tid == 0) {
            /* Call the offloading function from within OpenMP parallel region */
            process_on_gpu(a, b, c, N, alpha, dynamic_count);
        }
    }
    
    /* Compute reference on host */
    compute_reference(a, b, ref, N, alpha);
    
    /* Apply second stage transformations to reference */
    for (int i = 0; i < dynamic_count; i++) {
        if (i % 3 == 0) {
            ref[i] = sqrtf(ref[i]);
        } else if (i % 3 == 1) {
            ref[i] = ref[i] * ref[i];
        } else {
            ref[i] = 1.0f / (ref[i] + 1.0f);
        }
    }
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    for (i = 0; i < N; i++) {
        if (fabsf(c[i] - ref[i]) > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Mismatch at index %d: device=%f, host=%f\n", 
                       i, c[i], ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All %d elements match within tolerance %f\n", 
               N, tolerance);
    } else {
        printf("FAILURE: %d mismatches out of %d elements\n", errors, N);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(ref);
    
    return errors > 0 ? 1 : 0;
}
