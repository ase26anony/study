#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int n, int dynamic_n, float alpha) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(32)
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
        map(to: a[0:dynamic_n], c[0:dynamic_n]) map(from: d[0:dynamic_n]) \
        device(0) num_teams(16) thread_limit(128) simdlen(16)
    for (int i = 0; i < dynamic_n; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            d[i] = sqrtf(a[i] * a[i] + c[i] * c[i]);
        } else if (i % 3 == 1) {
            d[i] = a[i] * c[i] - alpha;
        } else {
            d[i] = a[i] + c[i] + alpha;
        }
    }
}

/* Host-side computation for validation */
static void compute_reference(float* a, float* b, float* c_ref, float* d_ref, 
                             int n, int dynamic_n, float alpha) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + b[i];
        } else {
            c_ref[i] = alpha * b[i] + a[i];
        }
    }
    
    for (int i = 0; i < dynamic_n; i++) {
        if (i % 3 == 0) {
            d_ref[i] = sqrtf(a[i] * a[i] + c_ref[i] * c_ref[i]);
        } else if (i % 3 == 1) {
            d_ref[i] = a[i] * c_ref[i] - alpha;
        } else {
            d_ref[i] = a[i] + c_ref[i] + alpha;
        }
    }
}

/* Validation function */
static int validate_results(float* dev, float* ref, int size, const char* name) {
    float eps = 1e-5f;
    int errors = 0;
    
    for (int i = 0; i < size; i++) {
        if (fabsf(dev[i] - ref[i]) > eps) {
            if (errors < 5) {
                printf("Mismatch at %s[%d]: device=%f, host=%f\n", 
                       name, i, dev[i], ref[i]);
            }
            errors++;
        }
    }
    
    if (errors > 0) {
        printf("%s: %d errors found\n", name, errors);
    }
    return errors;
}

int main(int argc, char** argv) {
    float *a, *b, *c, *d;
    float *c_ref, *d_ref;
    int dynamic_n = M;
    float alpha = 2.5f;
    
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
        a[i] = (float)(i + 1) * 0.1f;
        b[i] = (float)(N - i) * 0.05f;
        c[i] = 0.0f;
        d[i] = 0.0f;
        c_ref[i] = 0.0f;
        d_ref[i] = 0.0f;
    }
    
    /* Host-side OpenMP parallel region calling the offloading function */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the offloading function with different data slices */
        int chunk = N / 4;
        int start = tid * chunk;
        int end = (tid == 3) ? N : start + chunk;
        
        /* Call the function containing target regions */
        process_on_gpu(&a[start], &b[start], &c[start], &d[start], 
                      end - start, 
                      (dynamic_n - start > 0) ? dynamic_n - start : 0,
                      alpha * (tid + 1));
    }
    
    /* Compute reference on host */
    compute_reference(a, b, c_ref, d_ref, N, dynamic_n, alpha);
    
    /* Validate results */
    int total_errors = 0;
    total_errors += validate_results(c, c_ref, N, "c");
    total_errors += validate_results(d, d_ref, dynamic_n, "d");
    
    if (total_errors == 0) {
        printf("SUCCESS: All computations match between device and host\n");
        
        /* Print some sample values */
        printf("Sample values (first 5):\n");
        for (int i = 0; i < 5; i++) {
            printf("c[%d] = %f, d[%d] = %f\n", i, c[i], i, d[i]);
        }
    } else {
        printf("FAILURE: Found %d total errors\n", total_errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(c_ref); free(d_ref);
    
    return total_errors == 0 ? 0 : 1;
}
