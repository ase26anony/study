#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int n, float alpha, float beta) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
                device(0) num_teams(64) thread_limit(256) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = alpha * a[i] + beta * b[i];
        } else {
            c[i] = alpha * b[i] + beta * a[i];
        }
        
        /* Additional computation to prevent optimization */
        c[i] += sinf((float)i * 0.01f);
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:n], b[0:n]) map(tofrom: d[0:n]) \
                device(0) num_teams(32) thread_limit(128) simdlen(16)
    for (int i = 0; i < n; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            d[i] = a[i] * b[i] + c[i % N];
        } else if (i % 3 == 1) {
            d[i] = a[i] / (b[i] + 1.0f) - c[i % N];
        } else {
            d[i] = sqrtf(fabsf(a[i] - b[i])) * c[i % N];
        }
        
        /* Prevent dead code elimination */
        if (d[i] > 1000.0f) {
            d[i] = 1000.0f;
        }
    }
    
    /* Third loop with reduction pattern */
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
                map(to: c[0:N]) map(tofrom: sum) \
                device(0) num_teams(16) thread_limit(64) simdlen(8) \
                reduction(+:sum)
    for (int i = 0; i < N; i++) {
        sum += c[i] * (i % 10);
    }
    
    /* Use the result to prevent optimization */
    d[0] += sum * 0.001f;
}

/* Wrapper function called from host OpenMP parallel region */
static void compute_wrapper(float* a, float* b, float* c, float* d, 
                           int n, float alpha, float beta) {
    int thread_id = omp_get_thread_num();
    
    /* Add thread-specific offset to prevent identical computations */
    float offset = (float)thread_id * 0.1f;
    
    /* Call the GPU processing function */
    process_on_gpu(a, b, c, d, n, alpha + offset, beta + offset);
    
    /* Additional host-side computation */
    #pragma omp simd
    for (int i = 0; i < 100; i++) {
        d[i] += offset;
    }
}

int main() {
    float *a, *b, *c, *d;
    float *a_ref, *b_ref, *c_ref, *d_ref;
    float alpha = 2.5f, beta = 1.5f;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(N * sizeof(float));
    
    a_ref = (float*)malloc(N * sizeof(float));
    b_ref = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    d_ref = (float*)malloc(N * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(N - i) * 0.05f;
        c[i] = 0.0f;
        d[i] = 0.0f;
        
        a_ref[i] = a[i];
        b_ref[i] = b[i];
        c_ref[i] = c[i];
        d_ref[i] = d[i];
    }
    
    /* Host-side reference computation */
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a_ref[i] + beta * b_ref[i];
        } else {
            c_ref[i] = alpha * b_ref[i] + beta * a_ref[i];
        }
        c_ref[i] += sinf((float)i * 0.01f);
    }
    
    for (int i = 0; i < M; i++) {
        if (i % 3 == 0) {
            d_ref[i] = a_ref[i] * b_ref[i] + c_ref[i % N];
        } else if (i % 3 == 1) {
            d_ref[i] = a_ref[i] / (b_ref[i] + 1.0f) - c_ref[i % N];
        } else {
            d_ref[i] = sqrtf(fabsf(a_ref[i] - b_ref[i])) * c_ref[i % N];
        }
        if (d_ref[i] > 1000.0f) {
            d_ref[i] = 1000.0f;
        }
    }
    
    float sum_ref = 0.0f;
    for (int i = 0; i < N; i++) {
        sum_ref += c_ref[i] * (i % 10);
    }
    d_ref[0] += sum_ref * 0.001f;
    
    /* Host-side OpenMP parallel region calling GPU offloading */
    #pragma omp parallel num_threads(4)
    {
        compute_wrapper(a, b, c, d, M, alpha, beta);
    }
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    
    for (int i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
            if (errors <= 5) {
                printf("Mismatch at c[%d]: %f != %f\n", i, c[i], c_ref[i]);
            }
        }
    }
    
    for (int i = 0; i < M; i++) {
        if (fabsf(d[i] - d_ref[i]) > tolerance) {
            errors++;
            if (errors <= 10) {
                printf("Mismatch at d[%d]: %f != %f\n", i, d[i], d_ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All GPU computations match host reference\n");
        printf("Final values: c[0]=%f, d[0]=%f, d[%d]=%f\n", 
               c[0], d[0], M-1, d[M-1]);
    } else {
        printf("FAILURE: %d errors found\n", errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(a_ref); free(b_ref); free(c_ref); free(d_ref);
    
    return errors > 0 ? 1 : 0;
}
