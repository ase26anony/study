#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int n, int dynamic_count, float alpha) {
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
        
        /* Additional computation to prevent optimization */
        c[i] += sinf((float)i * 0.01f);
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_count], c[0:dynamic_count]) map(from: d[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int j = 0; j < dynamic_count; j++) {
        /* Different conditional pattern */
        if (j % 3 == 0) {
            d[j] = sqrtf(fabsf(a[j])) + c[j];
        } else if (j % 3 == 1) {
            d[j] = logf(fabsf(c[j]) + 1.0f) * a[j];
        } else {
            d[j] = (a[j] * c[j]) / (fabsf(c[j]) + 0.5f);
        }
        
        /* Prevent dead code elimination */
        if (d[j] > 100.0f) {
            d[j] = 100.0f;
        }
    }
}

/* Wrapper function called from host-side OpenMP parallel region */
static void compute_wrapper(float* a, float* b, float* c, float* d, 
                           int n, int dynamic_count) {
    float alpha = 2.5f;
    
    /* Call the GPU processing function */
    process_on_gpu(a, b, c, d, n, dynamic_count, alpha);
    
    /* Additional host-side computation to ensure function isn't optimized away */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        c[i] += 0.001f * (float)i;
    }
}

int main() {
    float *a, *b, *c, *d;
    float *a_ref, *b_ref, *c_ref, *d_ref;
    int i, dynamic_count;
    
    /* Allocate arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(N * sizeof(float));
    
    a_ref = (float*)malloc(N * sizeof(float));
    b_ref = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    d_ref = (float*)malloc(N * sizeof(float));
    
    /* Initialize data */
    #pragma omp parallel for
    for (i = 0; i < N; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(N - i) * 0.05f;
        c[i] = 0.0f;
        d[i] = 0.0f;
        
        a_ref[i] = a[i];
        b_ref[i] = b[i];
        c_ref[i] = c[i];
        d_ref[i] = d[i];
    }
    
    /* Dynamic count based on input */
    dynamic_count = M + (N % 100);
    
    /* Host-side OpenMP parallel region containing target offload */
    #pragma omp parallel num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread processes a different slice */
        int slice_size = N / 4;
        int start = thread_id * slice_size;
        int end = (thread_id == 3) ? N : start + slice_size;
        
        /* Adjust sizes for this thread's slice */
        int local_n = end - start;
        int local_dynamic = dynamic_count / 4;
        
        compute_wrapper(&a[start], &b[start], &c[start], &d[start], 
                       local_n, local_dynamic);
    }
    
    /* Compute reference on host for validation */
    float alpha = 2.5f;
    for (i = 0; i < N; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a_ref[i] + b_ref[i];
        } else {
            c_ref[i] = alpha * b_ref[i] + a_ref[i];
        }
        c_ref[i] += sinf((float)i * 0.01f);
        c_ref[i] += 0.001f * (float)i;
    }
    
    for (int j = 0; j < dynamic_count; j++) {
        if (j % 3 == 0) {
            d_ref[j] = sqrtf(fabsf(a_ref[j])) + c_ref[j];
        } else if (j % 3 == 1) {
            d_ref[j] = logf(fabsf(c_ref[j]) + 1.0f) * a_ref[j];
        } else {
            d_ref[j] = (a_ref[j] * c_ref[j]) / (fabsf(c_ref[j]) + 0.5f);
        }
        
        if (d_ref[j] > 100.0f) {
            d_ref[j] = 100.0f;
        }
    }
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    
    for (i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at c[%d]: %f vs %f\n", i, c[i], c_ref[i]);
            }
        }
    }
    
    for (i = 0; i < dynamic_count; i++) {
        if (fabsf(d[i] - d_ref[i]) > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Mismatch at d[%d]: %f vs %f\n", i, d[i], d_ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All computations match reference!\n");
    } else {
        printf("FAILURE: Found %d errors\n", errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(a_ref); free(b_ref); free(c_ref); free(d_ref);
    
    return errors > 0 ? 1 : 0;
}
