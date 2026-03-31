#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float scalar, int dynamic_n, int offset) {
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
        
        /* Additional computation to prevent optimization */
        c[i] += sinf((float)i * 0.01f);
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[offset:dynamic_n], b[offset:dynamic_n]) \
                map(tofrom: c[offset:dynamic_n]) \
                device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = offset; i < offset + dynamic_n; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            c[i] = c[i] * 0.5f + a[i] * b[i];
        } else if (i % 3 == 1) {
            c[i] = c[i] * 0.75f + sqrtf(fabsf(a[i]));
        } else {
            c[i] = c[i] * 1.25f + logf(fabsf(b[i]) + 1.0f);
        }
    }
}

/* Host-side reference computation */
static void compute_reference(float* a, float* b, float* ref, float scalar, int n) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            ref[i] = a[i] * scalar + b[i];
        } else {
            ref[i] = a[i] * (scalar * 0.5f) + b[i] * 2.0f;
        }
        ref[i] += sinf((float)i * 0.01f);
    }
}

int main() {
    float *a, *b, *c, *ref;
    int i;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    ref = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c || !ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    #pragma omp parallel for
    for (i = 0; i < N; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(N - i) * 0.05f;
        c[i] = 0.0f;
        ref[i] = 0.0f;
    }
    
    float scalar = 2.5f;
    int dynamic_n = M;  /* Dynamic iteration count */
    int offset = 100;   /* Offset for second loop */
    
    /* Host-side OpenMP parallel region calling the offloading function */
    #pragma omp parallel num_threads(4)
    {
        /* Each thread calls the GPU offloading function */
        process_on_gpu(a, b, c, scalar, dynamic_n, offset);
    }
    
    /* Compute reference on host */
    compute_reference(a, b, ref, scalar, N);
    
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
        printf("SUCCESS: All %d elements computed correctly on GPU\n", N);
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
