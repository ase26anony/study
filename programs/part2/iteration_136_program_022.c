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
        device(0) num_teams(64) thread_limit(256) simdlen(32)
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
        map(tofrom: d[0:dynamic_n]) \
        device(0) num_teams(32) thread_limit(128) simdlen(16)
    for (int j = 0; j < dynamic_n; j++) {
        /* Different computation pattern */
        if (j % 3 == 0) {
            d[j] = sqrtf(d[j]) + 1.0f;
        } else if (j % 3 == 1) {
            d[j] = d[j] * d[j] - 2.0f;
        } else {
            d[j] = sinf(d[j]) * cosf(d[j]);
        }
    }
}

/* Host-side computation for verification */
static void compute_reference(float* a, float* b, float* c_ref, 
                             float* d_ref, int n, int dynamic_n, float alpha) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + b[i];
        } else {
            c_ref[i] = alpha * b[i] + a[i];
        }
    }
    
    for (int j = 0; j < dynamic_n; j++) {
        if (j % 3 == 0) {
            d_ref[j] = sqrtf(d_ref[j]) + 1.0f;
        } else if (j % 3 == 1) {
            d_ref[j] = d_ref[j] * d_ref[j] - 2.0f;
        } else {
            d_ref[j] = sinf(d_ref[j]) * cosf(d_ref[j]);
        }
    }
}

/* Verification function */
static int verify_results(float* c, float* c_ref, float* d, float* d_ref, 
                         int n, int dynamic_n, float tolerance) {
    int errors = 0;
    
    for (int i = 0; i < n; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
            if (errors <= 5) {
                printf("Mismatch at c[%d]: %f != %f\n", i, c[i], c_ref[i]);
            }
        }
    }
    
    for (int j = 0; j < dynamic_n; j++) {
        if (fabsf(d[j] - d_ref[j]) > tolerance) {
            errors++;
            if (errors <= 5) {
                printf("Mismatch at d[%d]: %f != %f\n", j, d[j], d_ref[j]);
            }
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
    d = (float*)malloc(dynamic_n * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    d_ref = (float*)malloc(dynamic_n * sizeof(float));
    
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a[i] = (float)i / N;
        b[i] = (float)(N - i) / N;
        c[i] = 0.0f;
        c_ref[i] = 0.0f;
    }
    
    #pragma omp parallel for simd
    for (int j = 0; j < dynamic_n; j++) {
        d[j] = (float)j / dynamic_n;
        d_ref[j] = d[j];
    }
    
    /* Host-side reference computation */
    compute_reference(a, b, c_ref, d_ref, N, dynamic_n, 2.5f);
    
    /* Reset device arrays */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        c[i] = 0.0f;
    }
    
    #pragma omp parallel for simd
    for (int j = 0; j < dynamic_n; j++) {
        d[j] = (float)j / dynamic_n;
    }
    
    /* Outer host-side OpenMP parallel region */
    #pragma omp parallel num_threads(4)
    {
        /* Each thread calls the GPU processing function */
        #pragma omp master
        {
            printf("Starting GPU offloading from thread %d\n", omp_get_thread_num());
            process_on_gpu(a, b, c, d, N, dynamic_n, 2.5f);
        }
        
        #pragma omp barrier
        
        /* Additional host-side work in parallel */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            /* Dummy work to keep threads busy */
            volatile float x = sinf((float)i);
        }
    }
    
    /* Verify results */
    int errors = verify_results(c, c_ref, d, d_ref, N, dynamic_n, 1e-5f);
    
    if (errors == 0) {
        printf("SUCCESS: All GPU computations match reference!\n");
        
        /* Print some sample values */
        printf("Sample values (first 5):\n");
        for (int i = 0; i < 5; i++) {
            printf("c[%d] = %f, d[%d] = %f\n", i, c[i], i, d[i]);
        }
    } else {
        printf("FAILURE: Found %d mismatches\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(c_ref);
    free(d_ref);
    
    return errors == 0 ? 0 : 1;
}
