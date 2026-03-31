#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <assert.h>

#define N 10000
#define CHUNK_SIZE 256

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    int n, float scale, int dynamic_count) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        device(0) num_teams((n+CHUNK_SIZE-1)/CHUNK_SIZE) thread_limit(256) \
        simdlen(32) private(scale)
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
        device(0) num_teams(32) thread_limit(128) \
        simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* Another conditional with different pattern */
        if (i % 3 == 0) {
            c[i] = sqrtf(fabsf(c[i])) + 1.0f;
        } else if (i % 3 == 1) {
            c[i] = c[i] * c[i];
        } else {
            c[i] = 1.0f / (fabsf(c[i]) + 0.001f);
        }
    }
}

/* Host-side reference computation for validation */
static void compute_reference(float* a, float* b, float* ref, int n, float scale, int dynamic_count) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            ref[i] = a[i] * scale + b[i];
        } else {
            ref[i] = a[i] * (scale * 0.5f) + b[i] * 2.0f;
        }
    }
    
    for (int i = 0; i < dynamic_count; i++) {
        if (i % 3 == 0) {
            ref[i] = sqrtf(fabsf(ref[i])) + 1.0f;
        } else if (i % 3 == 1) {
            ref[i] = ref[i] * ref[i];
        } else {
            ref[i] = 1.0f / (fabsf(ref[i]) + 0.001f);
        }
    }
}

int main(int argc, char** argv) {
    float *a, *b, *c, *ref;
    int dynamic_count = (argc > 1) ? atoi(argv[1]) : 5000;
    
    if (dynamic_count > N) dynamic_count = N;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    ref = (float*)malloc(N * sizeof(float));
    
    assert(a && b && c && ref);
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 1) % 100) * 0.2f;
        c[i] = 0.0f;
        ref[i] = 0.0f;
    }
    
    float scale = 2.5f;
    
    /* Host-side OpenMP parallel region wrapping the GPU call */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the GPU function with different data slices */
        #pragma omp for schedule(static)
        for (int slice = 0; slice < 4; slice++) {
            int start = slice * (N / 4);
            int end = (slice == 3) ? N : (slice + 1) * (N / 4);
            int slice_size = end - start;
            
            /* Call GPU function - this creates nested parallelism */
            process_on_gpu(&a[start], &b[start], &c[start], 
                          slice_size, scale + tid * 0.1f, 
                          dynamic_count / 4);
        }
    }
    
    /* Compute reference on host */
    compute_reference(a, b, ref, N, scale, dynamic_count);
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    
    for (int i = 0; i < dynamic_count; i++) {
        if (fabsf(c[i] - ref[i]) > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Mismatch at index %d: GPU=%f, CPU=%f\n", 
                       i, c[i], ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All %d elements match within tolerance %e\n", 
               dynamic_count, tolerance);
    } else {
        printf("FAILURE: %d/%d elements differ\n", errors, dynamic_count);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(ref);
    
    return (errors == 0) ? 0 : 1;
}
