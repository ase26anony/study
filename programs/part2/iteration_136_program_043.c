#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define VALIDATION_TOL 1e-6

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    int n, float alpha, int dynamic_count) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n], alpha) map(from: c[0:n]) \
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

int main() {
    float *a, *b, *c, *ref;
    int i;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    ref = (float*)malloc(N * sizeof(float));
    
    #pragma omp parallel for simd
    for (i = 0; i < N; i++) {
        a[i] = (float)i / N;
        b[i] = (float)(N - i) / N;
        c[i] = 0.0f;
        ref[i] = 0.0f;
    }
    
    float alpha = 2.5f;
    int dynamic_count = N / 2;  /* Dynamic iteration count */
    
    /* Host-side OpenMP parallel region calling the offloading function */
    #pragma omp parallel num_threads(4)
    {
        /* Each thread calls the GPU offloading function */
        process_on_gpu(a, b, c, N, alpha, dynamic_count);
    }
    
    /* Compute reference on host */
    compute_reference(a, b, ref, N, alpha);
    
    /* Apply second transformation on host for reference */
    for (i = 0; i < dynamic_count; i++) {
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
    #pragma omp parallel for reduction(+:errors)
    for (i = 0; i < dynamic_count; i++) {
        if (fabsf(c[i] - ref[i]) > VALIDATION_TOL) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: GPU computation matches reference (0 errors)\n");
        
        /* Additional test with different parameters */
        printf("Running additional test with different parameters...\n");
        float *d = (float*)malloc(N * sizeof(float));
        
        #pragma omp parallel for
        for (i = 0; i < N; i++) {
            d[i] = sinf((float)i);
        }
        
        /* Nested parallelism with different clause combinations */
        #pragma omp parallel
        {
            #pragma omp target teams distribute parallel for simd \
                map(to: d[0:N]) map(from: c[0:N]) \
                device(0) num_teams(128) thread_limit(64) simdlen(8) \
                collapse(1)
            for (i = 0; i < N; i++) {
                /* Complex conditional to stress SIMT transformation */
                if ((i & 0x1F) == 0) {
                    c[i] = d[i] * d[i];
                } else if ((i & 0x1F) < 16) {
                    c[i] = d[i] + d[N - i - 1];
                } else {
                    c[i] = expf(d[i]);
                }
            }
        }
        
        free(d);
        printf("Additional test completed.\n");
    } else {
        printf("FAILURE: %d errors found\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(ref);
    
    return 0;
}
