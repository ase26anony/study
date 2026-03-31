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
                device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < n; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = alpha * a[i] + b[i];
        } else {
            c[i] = alpha * a[i] - b[i];
        }
        
        /* Additional computation to prevent optimization */
        c[i] = c[i] * (1.0f + 0.001f * (i % 10));
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:dynamic_count]) map(tofrom: c[0:dynamic_count]) \
                device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* Different conditional pattern */
        if (i < dynamic_count / 2) {
            c[i] = sqrtf(fabsf(c[i])) + 0.5f;
        } else {
            c[i] = c[i] * c[i] * 0.1f;
        }
        
        /* Nested condition to increase complexity */
        if (i % 3 == 0) {
            c[i] += 1.0f;
        } else if (i % 3 == 1) {
            c[i] -= 0.5f;
        }
    }
}

/* Host-side reference computation for validation */
static void compute_reference(float* a, float* b, float* ref, int n, float alpha) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            ref[i] = alpha * a[i] + b[i];
        } else {
            ref[i] = alpha * a[i] - b[i];
        }
        ref[i] = ref[i] * (1.0f + 0.001f * (i % 10));
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
    
    for (i = 0; i < N; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 1) % 100) * 0.2f;
        c[i] = 0.0f;
        ref[i] = 0.0f;
    }
    
    float alpha = 2.5f;
    int dynamic_count = M;  /* Dynamic iteration count */
    
    /* Host-side OpenMP parallel region wrapping the GPU call */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the GPU processing function */
        #pragma omp master
        {
            printf("Starting GPU computation from thread %d\n", tid);
            process_on_gpu(a, b, c, N, alpha, dynamic_count);
        }
        
        #pragma omp barrier
        
        /* Host-side computation for verification */
        #pragma omp for simd
        for (i = 0; i < N; i++) {
            if (i % 2 == 0) {
                ref[i] = alpha * a[i] + b[i];
            } else {
                ref[i] = alpha * a[i] - b[i];
            }
            ref[i] = ref[i] * (1.0f + 0.001f * (i % 10));
        }
    }
    
    /* Apply second phase computation on host for reference */
    for (i = 0; i < dynamic_count; i++) {
        if (i < dynamic_count / 2) {
            ref[i] = sqrtf(fabsf(ref[i])) + 0.5f;
        } else {
            ref[i] = ref[i] * ref[i] * 0.1f;
        }
        
        if (i % 3 == 0) {
            ref[i] += 1.0f;
        } else if (i % 3 == 1) {
            ref[i] -= 0.5f;
        }
    }
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    
    for (i = 0; i < dynamic_count; i++) {
        if (fabsf(c[i] - ref[i]) > tolerance) {
            errors++;
            if (errors <= 5) {
                printf("Mismatch at index %d: GPU=%f, CPU=%f\n", 
                       i, c[i], ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All %d elements computed correctly on GPU\n", dynamic_count);
    } else {
        printf("FAILURE: %d errors found in %d elements\n", errors, dynamic_count);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(ref);
    
    return errors > 0 ? 1 : 0;
}
