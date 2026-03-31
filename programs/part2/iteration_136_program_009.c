#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <string.h>

#define N 10000
#define M 5000
#define VALIDATE_TOL 1e-6

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int n, int dynamic_n, float alpha) {
    /* First target region: compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
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
    
    /* Second target region: dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_n], c[0:dynamic_n]) map(from: d[0:dynamic_n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int j = 0; j < dynamic_n; j++) {
        /* Different conditional pattern */
        if (j % 3 == 0) {
            d[j] = sqrtf(fabsf(a[j] * c[j])) + 1.0f;
        } else if (j % 3 == 1) {
            d[j] = logf(fabsf(c[j]) + 1.0f) * a[j];
        } else {
            d[j] = expf(a[j] * 0.01f) + c[j];
        }
        
        /* Nested conditionals increase complexity */
        if (d[j] > 100.0f) {
            d[j] = 100.0f;
        } else if (d[j] < -100.0f) {
            d[j] = -100.0f;
        }
    }
}

/* Host-side reference computation for validation */
static void compute_reference(float* a, float* b, float* c_ref, float* d_ref, 
                             int n, int dynamic_n, float alpha) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + b[i];
        } else {
            c_ref[i] = alpha * b[i] + a[i];
        }
        c_ref[i] += sinf((float)i * 0.01f);
    }
    
    for (int j = 0; j < dynamic_n; j++) {
        if (j % 3 == 0) {
            d_ref[j] = sqrtf(fabsf(a[j] * c_ref[j])) + 1.0f;
        } else if (j % 3 == 1) {
            d_ref[j] = logf(fabsf(c_ref[j]) + 1.0f) * a[j];
        } else {
            d_ref[j] = expf(a[j] * 0.01f) + c_ref[j];
        }
        
        if (d_ref[j] > 100.0f) {
            d_ref[j] = 100.0f;
        } else if (d_ref[j] < -100.0f) {
            d_ref[j] = -100.0f;
        }
    }
}

/* Validation function */
static int validate_results(float* dev, float* ref, int size, const char* name) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (fabsf(dev[i] - ref[i]) > VALIDATE_TOL) {
            if (errors < 5) {
                printf("Mismatch at %s[%d]: device=%.6f, host=%.6f\n", 
                       name, i, dev[i], ref[i]);
            }
            errors++;
        }
    }
    return errors;
}

int main(int argc, char** argv) {
    float *a, *b, *c, *d;
    float *c_ref, *d_ref;
    int dynamic_n = M;
    
    if (argc > 1) {
        dynamic_n = atoi(argv[1]);
        if (dynamic_n <= 0 || dynamic_n > N) dynamic_n = M;
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
        a[i] = (float)rand() / RAND_MAX * 10.0f - 5.0f;
        b[i] = (float)rand() / RAND_MAX * 10.0f - 5.0f;
    }
    
    float alpha = 2.5f;
    
    /* Host-side OpenMP parallel region wrapping the offload call */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Only thread 0 performs the GPU offload to avoid data races */
        if (thread_id == 0) {
            process_on_gpu(a, b, c, d, N, dynamic_n, alpha);
        }
        
        #pragma omp barrier
        
        /* All threads participate in host-side computation for validation */
        #pragma omp for
        for (int i = 0; i < N; i++) {
            /* Some dummy computation to keep threads busy */
            float temp = a[i] * b[i];
            (void)temp; /* Prevent unused variable warning */
        }
    }
    
    /* Compute reference on host */
    compute_reference(a, b, c_ref, d_ref, N, dynamic_n, alpha);
    
    /* Validate results */
    int c_errors = validate_results(c, c_ref, N, "c");
    int d_errors = validate_results(d, d_ref, dynamic_n, "d");
    
    if (c_errors == 0 && d_errors == 0) {
        printf("SUCCESS: All GPU computations match host reference\n");
        printf("First target region (constant n=%d): PASS\n", N);
        printf("Second target region (dynamic n=%d): PASS\n", dynamic_n);
    } else {
        printf("FAILURE: Found %d errors in array c, %d errors in array d\n", 
               c_errors, d_errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(c_ref); free(d_ref);
    
    return (c_errors == 0 && d_errors == 0) ? 0 : 1;
}
