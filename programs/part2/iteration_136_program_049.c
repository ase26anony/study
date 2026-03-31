#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000
#define VALIDATE_EPSILON 1e-6

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline,noipa))
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int dynamic_n, float alpha, float beta)
{
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
                device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = alpha * a[i] + beta * b[i];
        } else {
            c[i] = alpha * a[i] - beta * b[i];
        }
        
        /* Additional computation to prevent optimization */
        if (c[i] < 0.0f) {
            c[i] = fabsf(c[i]);
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:dynamic_n], b[0:dynamic_n]) map(from: d[0:dynamic_n]) \
                device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_n; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            d[i] = a[i] * b[i] + alpha;
        } else if (i % 3 == 1) {
            d[i] = a[i] / (b[i] + 1.0f) - beta;
        } else {
            d[i] = sqrtf(fabsf(a[i] - b[i]));
        }
        
        /* Nested condition to increase complexity */
        if (d[i] > 100.0f) {
            d[i] = 100.0f;
        } else if (d[i] < -100.0f) {
            d[i] = -100.0f;
        }
    }
}

/* Host-side reference computation for validation */
static void compute_reference(float* a, float* b, float* c_ref, float* d_ref,
                             int n, int dynamic_n, float alpha, float beta)
{
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + beta * b[i];
        } else {
            c_ref[i] = alpha * a[i] - beta * b[i];
        }
        
        if (c_ref[i] < 0.0f) {
            c_ref[i] = fabsf(c_ref[i]);
        }
    }
    
    for (int i = 0; i < dynamic_n; i++) {
        if (i % 3 == 0) {
            d_ref[i] = a[i] * b[i] + alpha;
        } else if (i % 3 == 1) {
            d_ref[i] = a[i] / (b[i] + 1.0f) - beta;
        } else {
            d_ref[i] = sqrtf(fabsf(a[i] - b[i]));
        }
        
        if (d_ref[i] > 100.0f) {
            d_ref[i] = 100.0f;
        } else if (d_ref[i] < -100.0f) {
            d_ref[i] = -100.0f;
        }
    }
}

/* Validation function */
static int validate_results(float* dev, float* ref, int n, const char* name)
{
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (fabsf(dev[i] - ref[i]) > VALIDATE_EPSILON) {
            if (errors < 5) {
                printf("  Mismatch at %s[%d]: device=%f, host=%f\n", 
                       name, i, dev[i], ref[i]);
            }
            errors++;
        }
    }
    return errors;
}

int main(int argc, char* argv[])
{
    float *a, *b, *c, *d, *c_ref, *d_ref;
    int dynamic_n = M;
    
    if (argc > 1) {
        dynamic_n = atoi(argv[1]);
        if (dynamic_n < 1 || dynamic_n > N) {
            dynamic_n = M;
        }
    }
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    d_ref = (float*)malloc(N * sizeof(float));
    
    srand(42);
    for (int i = 0; i < N; i++) {
        a[i] = (float)rand() / RAND_MAX * 10.0f;
        b[i] = (float)rand() / RAND_MAX * 10.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    float alpha = 2.5f;
    float beta = 1.5f;
    
    /* Host-side OpenMP parallel region wrapping the GPU call */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the GPU function with different data sections */
        #pragma omp for schedule(static)
        for (int t = 0; t < 4; t++) {
            /* Create thread-local copies of parameters */
            float local_alpha = alpha + 0.1f * tid;
            float local_beta = beta - 0.05f * tid;
            
            /* Call the GPU processing function from within OpenMP parallel region */
            process_on_gpu(a + t * (N/4), b + t * (N/4), 
                          c + t * (N/4), d + t * (N/4),
                          dynamic_n/4, local_alpha, local_beta);
        }
    }
    
    /* Compute reference on host */
    compute_reference(a, b, c_ref, d_ref, N, dynamic_n, alpha, beta);
    
    /* Validate results */
    printf("Validating GPU offload results...\n");
    
    int c_errors = validate_results(c, c_ref, N, "c");
    int d_errors = validate_results(d, d_ref, dynamic_n, "d");
    
    if (c_errors == 0 && d_errors == 0) {
        printf("SUCCESS: All GPU computations match host reference!\n");
        
        /* Additional verification: check that computation actually happened */
        float c_sum = 0.0f, d_sum = 0.0f;
        for (int i = 0; i < N; i++) c_sum += c[i];
        for (int i = 0; i < dynamic_n; i++) d_sum += d[i];
        printf("  Array c sum: %f\n", c_sum);
        printf("  Array d sum: %f\n", d_sum);
    } else {
        printf("FAILURE: Found %d errors in array c, %d errors in array d\n", 
               c_errors, d_errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(c_ref); free(d_ref);
    
    return (c_errors == 0 && d_errors == 0) ? 0 : 1;
}
