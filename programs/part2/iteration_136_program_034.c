#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* a, float* b, float* c, int n, float scale, int dynamic_count) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        device(0) num_teams(64) thread_limit(256) simdlen(32)
    for (int i = 0; i < n; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = a[i] * scale + b[i];
        } else {
            c[i] = a[i] * (scale * 0.5f) + b[i] * 2.0f;
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: c[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(128) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* More complex conditional with math function */
        if (i % 3 == 0) {
            c[i] = c[i] * sinf((float)i * 0.01f);
        } else if (i % 3 == 1) {
            c[i] = c[i] + cosf((float)i * 0.005f);
        } else {
            c[i] = sqrtf(fabsf(c[i]));
        }
    }
    
    /* Third loop with reduction pattern */
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
        map(to: c[0:n]) map(tofrom: sum) reduction(+:sum) \
        device(0) num_teams(16) thread_limit(64) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += c[i];
        /* Nested condition to increase complexity */
        if (c[i] > 100.0f) {
            c[i] = 100.0f;
        } else if (c[i] < -100.0f) {
            c[i] = -100.0f;
        }
    }
    
    printf("GPU computed sum: %f\n", sum);
}

/* Host-side reference computation */
static void compute_reference(float* a, float* b, float* c_ref, int n, float scale, int dynamic_count) {
    /* First computation matching GPU */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = a[i] * scale + b[i];
        } else {
            c_ref[i] = a[i] * (scale * 0.5f) + b[i] * 2.0f;
        }
    }
    
    /* Second computation matching GPU */
    for (int i = 0; i < dynamic_count; i++) {
        if (i % 3 == 0) {
            c_ref[i] = c_ref[i] * sinf((float)i * 0.01f);
        } else if (i % 3 == 1) {
            c_ref[i] = c_ref[i] + cosf((float)i * 0.005f);
        } else {
            c_ref[i] = sqrtf(fabsf(c_ref[i]));
        }
    }
    
    /* Third computation with reduction */
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += c_ref[i];
        if (c_ref[i] > 100.0f) {
            c_ref[i] = 100.0f;
        } else if (c_ref[i] < -100.0f) {
            c_ref[i] = -100.0f;
        }
    }
    printf("Host computed sum: %f\n", sum);
}

int main(int argc, char** argv) {
    float* a = (float*)malloc(N * sizeof(float));
    float* b = (float*)malloc(N * sizeof(float));
    float* c = (float*)malloc(N * sizeof(float));
    float* c_ref = (float*)malloc(N * sizeof(float));
    
    /* Initialize arrays */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(N - i) * 0.05f;
        c[i] = 0.0f;
        c_ref[i] = 0.0f;
    }
    
    float scale = 2.5f;
    int dynamic_count = (argc > 1) ? atoi(argv[1]) : M;
    if (dynamic_count > N) dynamic_count = N;
    
    /* Host-side OpenMP parallel region calling the GPU function */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the GPU processing function */
        #pragma omp master
        {
            printf("Starting GPU computation from thread %d\n", tid);
            process_on_gpu(a, b, c, N, scale, dynamic_count);
        }
        
        #pragma omp barrier
        
        /* Host-side reference computation in parallel */
        #pragma omp for schedule(static)
        for (int chunk = 0; chunk < 4; chunk++) {
            int start = chunk * (N / 4);
            int end = (chunk == 3) ? N : (chunk + 1) * (N / 4);
            for (int i = start; i < end; i++) {
                if (i % 2 == 0) {
                    c_ref[i] = a[i] * scale + b[i];
                } else {
                    c_ref[i] = a[i] * (scale * 0.5f) + b[i] * 2.0f;
                }
            }
        }
    }
    
    /* Complete host reference computation */
    compute_reference(a, b, c_ref, N, scale, dynamic_count);
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    for (int i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Mismatch at index %d: GPU=%f, Host=%f\n", i, c[i], c_ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All %d elements match within tolerance %f\n", N, tolerance);
    } else {
        printf("FAILURE: %d elements differ beyond tolerance %f\n", errors, tolerance);
    }
    
    free(a);
    free(b);
    free(c);
    free(c_ref);
    
    return errors > 0 ? 1 : 0;
}
