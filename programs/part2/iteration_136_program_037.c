#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* data, float* result, int dynamic_n, float a, float b) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: data[0:N]) map(from: result[0:N]) \
        device(0) num_teams(32) thread_limit(256) simdlen(32)
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
        map(tofrom: result[0:dynamic_n]) \
        device(0) num_teams(16) thread_limit(128) simdlen(16)
    for (int i = 0; i < dynamic_n; i++) {
        /* More complex conditional logic */
        if (i % 3 == 0) {
            result[i] = sqrtf(fabsf(result[i]));
        } else if (i % 3 == 1) {
            result[i] = result[i] * result[i];
        } else {
            result[i] = 1.0f / (1.0f + fabsf(result[i]));
        }
    }
}

/* Another helper with different loop structure */
static __attribute__((noinline))
void process_reduction(float* data, int n, float* sum_result) {
    float sum = 0.0f;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: data[0:n]) map(tofrom: sum) \
        device(0) num_teams(8) thread_limit(64) simdlen(8) \
        reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Conditional reduction */
        if (data[i] > 0.5f) {
            sum += data[i] * 2.0f;
        } else {
            sum += data[i];
        }
    }
    
    *sum_result = sum;
}

int main() {
    float *data = (float*)malloc(N * sizeof(float));
    float *result = (float*)malloc(N * sizeof(float));
    float *host_result = (float*)malloc(N * sizeof(float));
    
    if (!data || !result || !host_result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        data[i] = (float)i / N;
        result[i] = 0.0f;
        host_result[i] = 0.0f;
    }
    
    float a = 2.5f, b = 1.5f;
    int dynamic_n = M;
    
    /* Host-side OpenMP parallel region calling GPU offloading function */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread calls the GPU offloading function */
        if (thread_id == 0) {
            process_on_gpu(data, result, dynamic_n, a, b);
        }
        
        #pragma omp barrier
        
        /* Another thread does reduction */
        if (thread_id == 1) {
            float sum;
            process_reduction(data, dynamic_n, &sum);
            printf("Thread %d: Reduction result = %f\n", thread_id, sum);
        }
    }
    
    /* Compute reference on host */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            host_result[i] = a * data[i] + b;
        } else {
            host_result[i] = a * data[i] - b;
        }
    }
    
    #pragma omp parallel for simd
    for (int i = 0; i < dynamic_n; i++) {
        if (i % 3 == 0) {
            host_result[i] = sqrtf(fabsf(host_result[i]));
        } else if (i % 3 == 1) {
            host_result[i] = host_result[i] * host_result[i];
        } else {
            host_result[i] = 1.0f / (1.0f + fabsf(host_result[i]));
        }
    }
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-5f;
    
    #pragma omp parallel for reduction(+:errors)
    for (int i = 0; i < dynamic_n; i++) {
        if (fabsf(result[i] - host_result[i]) > tolerance) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: GPU computation matches host reference\n");
    } else {
        printf("FAILURE: %d errors found\n", errors);
    }
    
    /* Cleanup */
    free(data);
    free(result);
    free(host_result);
    
    return errors > 0 ? 1 : 0;
}
