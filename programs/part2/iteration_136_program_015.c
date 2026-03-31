#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int dynamic_n, float alpha) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = a[i] * alpha + b[i];
        } else {
            c[i] = a[i] * 2.0f + b[i] * 0.5f;
        }
        
        /* Additional computation to prevent optimization */
        c[i] += sinf(i * 0.001f);
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: d[0:dynamic_n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int j = 0; j < dynamic_n; j++) {
        /* Different conditional pattern */
        if (j < dynamic_n / 2) {
            d[j] = c[j % N] * 1.5f;
        } else {
            d[j] = c[j % N] * 0.75f;
        }
        
        /* Complex conditional chain */
        if (j % 3 == 0) {
            d[j] += 1.0f;
        } else if (j % 3 == 1) {
            d[j] += 2.0f;
        } else {
            d[j] += 3.0f;
        }
    }
}

/* Host-side reference computation for validation */
static void compute_reference(float* a, float* b, float* c_ref, 
                             float* d_ref, int n, float alpha) {
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            c_ref[i] = a[i] * alpha + b[i];
        } else {
            c_ref[i] = a[i] * 2.0f + b[i] * 0.5f;
        }
        c_ref[i] += sinf(i * 0.001f);
    }
    
    for (int j = 0; j < n; j++) {
        if (j < n / 2) {
            d_ref[j] = c_ref[j % N] * 1.5f;
        } else {
            d_ref[j] = c_ref[j % N] * 0.75f;
        }
        
        if (j % 3 == 0) {
            d_ref[j] += 1.0f;
        } else if (j % 3 == 1) {
            d_ref[j] += 2.0f;
        } else {
            d_ref[j] += 3.0f;
        }
    }
}

int main(int argc, char** argv) {
    float *a, *b, *c, *d, *c_ref, *d_ref;
    float alpha = 2.5f;
    int dynamic_n = M;
    
    if (argc > 1) {
        dynamic_n = atoi(argv[1]);
        if (dynamic_n <= 0 || dynamic_n > 2*N) dynamic_n = M;
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
        d[j] = 0.0f;
        d_ref[j] = 0.0f;
    }
    
    /* Host-side OpenMP parallel region calling the offloading function */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the offloading function with different data slices */
        if (tid == 0) {
            process_on_gpu(a, b, c, d, dynamic_n, alpha);
        }
        
        #pragma omp barrier
        
        /* Other threads can do additional work */
        if (tid > 0) {
            #pragma omp simd
            for (int i = 0; i < 100; i++) {
                /* Dummy work to keep threads busy */
                volatile float x = sinf(tid + i);
            }
        }
    }
    
    /* Compute reference on host */
    compute_reference(a, b, c_ref, d_ref, dynamic_n, alpha);
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    
    #pragma omp parallel for reduction(+:errors)
    for (int i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
        }
    }
    
    #pragma omp parallel for reduction(+:errors)
    for (int j = 0; j < dynamic_n; j++) {
        if (fabsf(d[j] - d_ref[j]) > tolerance) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All %d+%d elements match within tolerance\n", N, dynamic_n);
        
        /* Print sample values for verification */
        printf("Sample values (first 5):\n");
        printf("c[0]=%.6f (ref=%.6f)\n", c[0], c_ref[0]);
        printf("c[1]=%.6f (ref=%.6f)\n", c[1], c_ref[1]);
        printf("d[0]=%.6f (ref=%.6f)\n", d[0], d_ref[0]);
        printf("d[%d]=%.6f (ref=%.6f)\n", dynamic_n-1, d[dynamic_n-1], d_ref[dynamic_n-1]);
    } else {
        printf("FAILURE: %d elements differ\n", errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(c_ref); free(d_ref);
    
    return errors == 0 ? 0 : 1;
}
