#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int n, int dynamic_count) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < n; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = a[i] * 2.5f + b[i];
        } else {
            c[i] = a[i] * 1.5f - b[i];
        }
        
        /* Additional computation to prevent optimization */
        c[i] += sinf((float)i * 0.01f);
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: d[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int j = 0; j < dynamic_count; j++) {
        /* Different conditional pattern */
        if (j % 3 == 0) {
            d[j] = d[j] * 3.0f + j * 0.1f;
        } else if (j % 3 == 1) {
            d[j] = d[j] * 2.0f - j * 0.05f;
        } else {
            d[j] = d[j] * 1.0f + j * 0.01f;
        }
        
        /* Complex math to ensure loop isn't optimized away */
        d[j] += cosf((float)j * 0.02f) * tanf((float)j * 0.001f);
    }
}

/* Host-side reference computation for validation */
static void compute_reference(float* a, float* b, float* c_ref, float* d_ref, 
                             int n, int dynamic_count) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = a[i] * 2.5f + b[i];
        } else {
            c_ref[i] = a[i] * 1.5f - b[i];
        }
        c_ref[i] += sinf((float)i * 0.01f);
    }
    
    for (int j = 0; j < dynamic_count; j++) {
        if (j % 3 == 0) {
            d_ref[j] = d_ref[j] * 3.0f + j * 0.1f;
        } else if (j % 3 == 1) {
            d_ref[j] = d_ref[j] * 2.0f - j * 0.05f;
        } else {
            d_ref[j] = d_ref[j] * 1.0f + j * 0.01f;
        }
        d_ref[j] += cosf((float)j * 0.02f) * tanf((float)j * 0.001f);
    }
}

int main() {
    float *a, *b, *c, *d, *c_ref, *d_ref;
    int dynamic_count = M;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(dynamic_count * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    d_ref = (float*)malloc(dynamic_count * sizeof(float));
    
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(N - i) * 0.05f;
        c[i] = 0.0f;
        c_ref[i] = 0.0f;
    }
    
    #pragma omp parallel for simd
    for (int j = 0; j < dynamic_count; j++) {
        d[j] = (float)j * 0.2f;
        d_ref[j] = d[j];
    }
    
    /* Host-side reference computation */
    compute_reference(a, b, c_ref, d_ref, N, dynamic_count);
    
    /* Reset device arrays */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        c[i] = 0.0f;
    }
    
    #pragma omp parallel for simd
    for (int j = 0; j < dynamic_count; j++) {
        d[j] = (float)j * 0.2f;
    }
    
    /* Nested parallelism: host OpenMP parallel region calling GPU offloading */
    #pragma omp parallel num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread calls the GPU offloading function */
        #pragma omp barrier
        
        if (thread_id == 0) {
            process_on_gpu(a, b, c, d, N, dynamic_count);
        }
        
        #pragma omp barrier
        
        /* Verify results on each thread */
        int errors = 0;
        #pragma omp for simd reduction(+:errors)
        for (int i = 0; i < N; i++) {
            if (fabsf(c[i] - c_ref[i]) > 1e-4f) {
                errors++;
            }
        }
        
        #pragma omp for simd reduction(+:errors)
        for (int j = 0; j < dynamic_count; j++) {
            if (fabsf(d[j] - d_ref[j]) > 1e-4f) {
                errors++;
            }
        }
        
        #pragma omp critical
        {
            if (errors > 0) {
                printf("Thread %d: Found %d errors in GPU computation\n", 
                       thread_id, errors);
            } else {
                printf("Thread %d: GPU computation verified successfully\n", 
                       thread_id);
            }
        }
    }
    
    /* Additional test with different dynamic count */
    int dynamic_count2 = 3000;
    float* e = (float*)malloc(dynamic_count2 * sizeof(float));
    
    #pragma omp parallel for simd
    for (int k = 0; k < dynamic_count2; k++) {
        e[k] = (float)k * 0.3f;
    }
    
    /* Call GPU function again with different parameter */
    #pragma omp parallel
    {
        process_on_gpu(a, b, c, e, N, dynamic_count2);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(c_ref);
    free(d_ref);
    free(e);
    
    printf("Program completed successfully\n");
    return 0;
}
