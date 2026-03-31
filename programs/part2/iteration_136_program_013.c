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
            c[i] = alpha * b[i] + a[i];
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_count], b[0:dynamic_count]) \
        map(tofrom: c[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            c[i] = sqrtf(a[i] * a[i] + b[i] * b[i]);
        } else if (i % 3 == 1) {
            c[i] = a[i] * b[i] - alpha;
        } else {
            c[i] = (a[i] + b[i]) * alpha;
        }
    }
}

/* Host-side reference computation for validation */
static void compute_reference(float* a, float* b, float* c_ref, 
                             int n, float alpha, int dynamic_count) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + b[i];
        } else {
            c_ref[i] = alpha * b[i] + a[i];
        }
    }
    
    for (int i = 0; i < dynamic_count; i++) {
        if (i % 3 == 0) {
            c_ref[i] = sqrtf(a[i] * a[i] + b[i] * b[i]);
        } else if (i % 3 == 1) {
            c_ref[i] = a[i] * b[i] - alpha;
        } else {
            c_ref[i] = (a[i] + b[i]) * alpha;
        }
    }
}

int main() {
    float *a, *b, *c, *c_ref;
    float alpha = 2.5f;
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
    
    /* Initialize with some pattern */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 50) % 100) * 0.2f;
        c[i] = 0.0f;
        c_ref[i] = 0.0f;
    }
    
    /* Host-side OpenMP parallel region calling the offloading function */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        /* Each thread processes a portion */
        int chunk_size = N / num_threads;
        int start = thread_id * chunk_size;
        int end = (thread_id == num_threads - 1) ? N : start + chunk_size;
        
        /* Call the offloading function from within host parallel region */
        process_on_gpu(&a[start], &b[start], &c[start], 
                      end - start, alpha, 
                      (dynamic_count - start) > 0 ? (dynamic_count - start) : 0);
    }
    
    /* Compute reference on host */
    compute_reference(a, b, c_ref, N, alpha, dynamic_count);
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-5f;
    
    for (int i = 0; i < dynamic_count; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
            if (errors <= 10) {
                printf("Mismatch at index %d: device=%f, host=%f\n", 
                       i, c[i], c_ref[i]);
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
    free(c_ref);
    
    return errors > 0 ? 1 : 0;
}
