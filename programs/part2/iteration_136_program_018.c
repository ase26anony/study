#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* data, float* result, int dynamic_count, float a, float b) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: data[0:N]) map(from: result[0:N]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            result[i] = a * data[i] + b;  /* SAXPY-like operation */
        } else {
            result[i] = a * data[i] - b;  /* Different computation for odd indices */
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: result[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* Another conditional with different pattern */
        if (i % 3 == 0) {
            result[i] = result[i] * 2.0f;
        } else if (i % 3 == 1) {
            result[i] = sqrtf(fabsf(result[i]));
        } else {
            result[i] = result[i] + 1.0f;
        }
    }
}

/* Host-side parallel region wrapper */
void host_parallel_wrapper(float* data, float* result, int dynamic_count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        /* Each thread calls the GPU processing function */
        #pragma omp master
        {
            process_on_gpu(data, result, dynamic_count, 2.5f, 1.5f);
        }
        #pragma omp barrier
        
        /* Host-side computation for verification */
        #pragma omp for simd
        for (int i = 0; i < N; i++) {
            /* Just to keep threads busy while master offloads */
            volatile float dummy = data[i] * tid;
            (void)dummy;
        }
    }
}

int main() {
    float* data = (float*)malloc(N * sizeof(float));
    float* result_gpu = (float*)malloc(N * sizeof(float));
    float* result_host = (float*)malloc(N * sizeof(float));
    
    if (!data || !result_gpu || !result_host) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data with pattern */
    for (int i = 0; i < N; i++) {
        data[i] = (float)i / 100.0f;
        result_gpu[i] = 0.0f;
        result_host[i] = 0.0f;
    }
    
    /* Dynamic count based on input */
    int dynamic_count = M;
    
    /* Call through host parallel wrapper */
    host_parallel_wrapper(data, result_gpu, dynamic_count);
    
    /* Compute reference on host */
    float a = 2.5f, b = 1.5f;
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            result_host[i] = a * data[i] + b;
        } else {
            result_host[i] = a * data[i] - b;
        }
    }
    
    for (int i = 0; i < dynamic_count; i++) {
        if (i % 3 == 0) {
            result_host[i] = result_host[i] * 2.0f;
        } else if (i % 3 == 1) {
            result_host[i] = sqrtf(fabsf(result_host[i]));
        } else {
            result_host[i] = result_host[i] + 1.0f;
        }
    }
    
    /* Verify results */
    int errors = 0;
    float tolerance = 1e-5f;
    for (int i = 0; i < N; i++) {
        if (fabsf(result_gpu[i] - result_host[i]) > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Mismatch at index %d: GPU=%f, Host=%f\n", 
                       i, result_gpu[i], result_host[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All results match within tolerance %e\n", tolerance);
    } else {
        printf("FAILURE: %d mismatches found\n", errors);
    }
    
    free(data);
    free(result_gpu);
    free(result_host);
    
    return errors > 0 ? 1 : 0;
}
