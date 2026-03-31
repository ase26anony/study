#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    int n, float scale, int dynamic_count) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(32)
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
        device(0) num_teams(16) thread_limit(128) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* Another conditional with different pattern */
        if (i % 3 == 0) {
            c[i] = sqrtf(fabsf(c[i])) + 1.0f;
        } else if (i % 3 == 1) {
            c[i] = c[i] * c[i];
        } else {
            c[i] = 1.0f / (1.0f + fabsf(c[i]));
        }
    }
}

/* Another helper with different loop structure */
static __attribute__((noinline))
void process_partial(float* restrict data, int start, int end, float param) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[start:end-start]) \
        device(0) num_teams(8) thread_limit(64) simdlen(8)
    for (int i = start; i < end; i++) {
        /* Complex conditional chain */
        if (data[i] > 0.0f) {
            data[i] = logf(data[i] + 1.0f) * param;
        } else if (data[i] < -1.0f) {
            data[i] = sinf(data[i]) * param;
        } else {
            data[i] = cosf(data[i]) * param;
        }
    }
}

int main() {
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *c_ref = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c || !c_ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a[i] = (float)i / 100.0f;
        b[i] = (float)(N - i) / 50.0f;
        c[i] = 0.0f;
        c_ref[i] = 0.0f;
    }
    
    float scale = 2.5f;
    int dynamic_count = M;  /* Dynamic iteration count */
    
    /* Host-side OpenMP parallel region calling target offloading function */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        int chunk = N / nthreads;
        int start = tid * chunk;
        int end = (tid == nthreads - 1) ? N : start + chunk;
        
        /* Each thread calls target offloading function */
        process_on_gpu(a + start, b + start, c + start, 
                      end - start, scale, dynamic_count / nthreads);
        
        /* Call another target function with different parameters */
        process_partial(c, start, end, 1.0f + tid * 0.1f);
    }
    
    /* Compute reference on host for validation */
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            c_ref[i] = a[i] * scale + b[i];
        } else {
            c_ref[i] = a[i] * (scale * 0.5f) + b[i] * 2.0f;
        }
    }
    
    #pragma omp parallel for
    for (int i = 0; i < M; i++) {
        if (i % 3 == 0) {
            c_ref[i] = sqrtf(fabsf(c_ref[i])) + 1.0f;
        } else if (i % 3 == 1) {
            c_ref[i] = c_ref[i] * c_ref[i];
        } else {
            c_ref[i] = 1.0f / (1.0f + fabsf(c_ref[i]));
        }
    }
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    
    #pragma omp parallel for reduction(+:errors)
    for (int i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: GPU computation matches reference (0 errors)\n");
        
        /* Additional test with nested parallelism */
        float *test_arr = (float*)malloc(1000 * sizeof(float));
        #pragma omp parallel for
        for (int i = 0; i < 1000; i++) {
            test_arr[i] = (float)i;
        }
        
        /* Nested target region */
        #pragma omp parallel
        {
            #pragma omp target teams distribute parallel for simd \
                map(tofrom: test_arr[0:1000]) \
                device(0) num_teams(4) thread_limit(32) simdlen(4)
            for (int i = 0; i < 1000; i++) {
                test_arr[i] = test_arr[i] * test_arr[i];
            }
        }
        
        free(test_arr);
    } else {
        printf("FAILURE: %d errors found\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(c_ref);
    
    return 0;
}
