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
        map(to: a[0:n], b[0:n], alpha) map(from: c[0:n]) \
        device(0) num_teams(64) thread_limit(128) simdlen(8)
    for (int i = 0; i < n; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = alpha * a[i] + b[i];
        } else {
            c[i] = alpha * a[i] - b[i];
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: c[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(4)
    for (int i = 0; i < dynamic_count; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            c[i] = sqrtf(fabsf(c[i]));
        } else if (i % 3 == 1) {
            c[i] = c[i] * c[i];
        } else {
            c[i] = 1.0f / (1.0f + fabsf(c[i]));
        }
    }
}

/* Host-side parallel function */
static void host_parallel_computation(float* a, float* b, float* c_ref, int n, float alpha) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + b[i];
        } else {
            c_ref[i] = alpha * a[i] - b[i];
        }
    }
}

int main() {
    float *a, *b, *c, *c_ref;
    int i;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c || !c_ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with some pattern */
    #pragma omp parallel for
    for (i = 0; i < N; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 1) % 100) * 0.2f;
        c[i] = 0.0f;
        c_ref[i] = 0.0f;
    }
    
    float alpha = 2.5f;
    int dynamic_count = M;  /* Dynamic iteration count for second loop */
    
    /* Host-side reference computation in parallel region */
    #pragma omp parallel
    {
        host_parallel_computation(a, b, c_ref, N, alpha);
    }
    
    /* Call the function with target offloading from within parallel region */
    #pragma omp parallel
    {
        #pragma omp single
        {
            process_on_gpu(a, b, c, N, alpha, dynamic_count);
        }
    }
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    
    for (i = 0; i < dynamic_count; i++) {
        float expected;
        if (i % 2 == 0) {
            expected = alpha * a[i] + b[i];
        } else {
            expected = alpha * a[i] - b[i];
        }
        
        if (i % 3 == 0) {
            expected = sqrtf(fabsf(expected));
        } else if (i % 3 == 1) {
            expected = expected * expected;
        } else {
            expected = 1.0f / (1.0f + fabsf(expected));
        }
        
        if (fabsf(c[i] - expected) > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Mismatch at i=%d: c=%f, expected=%f\n", i, c[i], expected);
            }
        }
    }
    
    /* Check remaining elements (should match first computation only) */
    for (i = dynamic_count; i < N; i++) {
        float expected;
        if (i % 2 == 0) {
            expected = alpha * a[i] + b[i];
        } else {
            expected = alpha * a[i] - b[i];
        }
        
        if (fabsf(c[i] - expected) > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Mismatch at i=%d: c=%f, expected=%f\n", i, c[i], expected);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All computations match!\n");
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
