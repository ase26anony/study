#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    int n, float scale, int dynamic_count) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
                device(0) num_teams(32) thread_limit(256) simdlen(32)
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
                map(tofrom: c[0:dynamic_count]) \
                device(0) num_teams(16) thread_limit(128) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            c[i] = sqrtf(fabsf(c[i]));
        } else if (i % 3 == 1) {
            c[i] = c[i] * c[i];
        } else {
            c[i] = c[i] + 1.0f;
        }
    }
}

/* Host-side computation for verification */
static void compute_reference(float* a, float* b, float* ref, int n, float scale) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            ref[i] = a[i] * scale + b[i];
        } else {
            ref[i] = a[i] * (scale * 0.5f) + b[i] * 2.0f;
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
        a[i] = (float)(i + 1) * 0.1f;
        b[i] = (float)(N - i) * 0.05f;
        c[i] = 0.0f;
        ref[i] = 0.0f;
    }
    
    float scale = 2.5f;
    int dynamic_count = (argc > 1) ? atoi(argv[1]) : M;
    if (dynamic_count > N) dynamic_count = N;
    
    /* Host-side OpenMP parallel region wrapping the GPU call */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        if (tid == 0) {
            /* Call the function containing target offloading regions */
            process_on_gpu(a, b, c, N, scale, dynamic_count);
        }
    }
    
    /* Compute reference on host */
    compute_reference(a, b, ref, N, scale);
    
    /* Apply second transformation on reference for verification */
    for (i = 0; i < dynamic_count; i++) {
        if (i % 3 == 0) {
            ref[i] = sqrtf(fabsf(ref[i]));
        } else if (i % 3 == 1) {
            ref[i] = ref[i] * ref[i];
        } else {
            ref[i] = ref[i] + 1.0f;
        }
    }
    
    /* Verify results */
    int errors = 0;
    float tolerance = 1e-4f;
    for (i = 0; i < dynamic_count; i++) {
        if (fabsf(c[i] - ref[i]) > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Mismatch at index %d: device=%f, host=%f\n", 
                       i, c[i], ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All %d elements computed correctly on GPU\n", dynamic_count);
    } else {
        printf("FAILURE: %d errors found\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(ref);
    
    return errors > 0 ? 1 : 0;
}
