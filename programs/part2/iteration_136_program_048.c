#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000
#define VALIDATE_TOL 1e-6

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int dynamic_n, float alpha, float beta) {
    /* First target region: compile-time constant iteration count */
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
        c[i] = c[i] * (1.0f + sinf(i * 0.01f));
    }
    
    /* Second target region: dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: d[0:dynamic_n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int j = 0; j < dynamic_n; j++) {
        /* Different conditional pattern */
        if (j < dynamic_n / 2) {
            d[j] = sqrtf(fabsf(d[j])) + j * 0.1f;
        } else {
            d[j] = logf(fabsf(d[j]) + 1.0f) - j * 0.05f;
        }
        
        /* Nested conditionals increase complexity */
        if (d[j] > 100.0f) {
            d[j] = 100.0f;
        } else if (d[j] < -100.0f) {
            d[j] = -100.0f;
        }
    }
}

/* Host-side reference computation */
static void compute_reference(float* a, float* b, float* c_ref, float* d_ref, 
                             int n, float alpha, float beta) {
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + beta * b[i];
        } else {
            c_ref[i] = alpha * a[i] - beta * b[i];
        }
        c_ref[i] = c_ref[i] * (1.0f + sinf(i * 0.01f));
    }
    
    for (int j = 0; j < n; j++) {
        if (j < n / 2) {
            d_ref[j] = sqrtf(fabsf(d_ref[j])) + j * 0.1f;
        } else {
            d_ref[j] = logf(fabsf(d_ref[j]) + 1.0f) - j * 0.05f;
        }
        
        if (d_ref[j] > 100.0f) {
            d_ref[j] = 100.0f;
        } else if (d_ref[j] < -100.0f) {
            d_ref[j] = -100.0f;
        }
    }
}

/* Validation function */
static int validate_results(float* dev, float* ref, int n, const char* name) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (fabsf(dev[i] - ref[i]) > VALIDATE_TOL) {
            if (errors < 5) {
                printf("  Mismatch at %s[%d]: device=%.6f, host=%.6f\n", 
                       name, i, dev[i], ref[i]);
            }
            errors++;
        }
    }
    return errors;
}

int main(int argc, char** argv) {
    float *a, *b, *c, *d, *c_ref, *d_ref;
    float alpha = 2.5f, beta = 1.5f;
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
    
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a[i] = (float)i / N;
        b[i] = (float)(N - i) / N;
        c[i] = 0.0f;
        d[i] = (float)(i % 100) - 50.0f;
        c_ref[i] = 0.0f;
        d_ref[i] = d[i];
    }
    
    /* Host-side OpenMP parallel region wrapping the GPU call */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the GPU function with different data sections */
        #pragma omp master
        {
            printf("Thread %d: Starting GPU offload...\n", tid);
            double start = omp_get_wtime();
            
            /* This call should trigger the SIMT transformation */
            process_on_gpu(a, b, c, d, dynamic_n, alpha, beta);
            
            double end = omp_get_wtime();
            printf("Thread %d: GPU offload completed in %.3f seconds\n", 
                   tid, end - start);
        }
        
        #pragma omp barrier
        
        /* Other threads do host computation */
        #pragma omp for
        for (int i = 0; i < 1000; i++) {
            /* Some dummy work */
            volatile float x = sinf(i * 0.1f);
        }
    }
    
    /* Compute reference on host */
    printf("Computing reference on host...\n");
    compute_reference(a, b, c_ref, d_ref, dynamic_n, alpha, beta);
    
    /* Validate results */
    printf("Validating results...\n");
    int c_errors = validate_results(c, c_ref, N, "c");
    int d_errors = validate_results(d, d_ref, dynamic_n, "d");
    
    if (c_errors == 0 && d_errors == 0) {
        printf("SUCCESS: All results match within tolerance %.6f\n", VALIDATE_TOL);
        
        /* Additional verification - check some values */
        printf("Sample values:\n");
        for (int i = 0; i < 5; i++) {
            printf("  c[%d] = %.6f, d[%d] = %.6f\n", 
                   i * 1000, c[i * 1000], i * 200, d[i * 200]);
        }
    } else {
        printf("FAILURE: c errors = %d, d errors = %d\n", c_errors, d_errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(c_ref); free(d_ref);
    
    return (c_errors == 0 && d_errors == 0) ? 0 : 1;
}
