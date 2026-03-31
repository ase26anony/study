#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* data, float* results, int dynamic_count, float alpha) {
    float local_data[N];
    
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:N]) map(from: results[0:N]) \
        device(0) num_teams(128) thread_limit(256) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            results[i] = data[i] * 2.0f;
        } else {
            results[i] = data[i] / 2.0f;
        }
        
        /* Additional computation to prevent optimization */
        results[i] += sinf(i * 0.01f);
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: local_data[0:dynamic_count]) \
        device(0) num_teams(64) thread_limit(128) simdlen(16)
    for (int j = 0; j < dynamic_count; j++) {
        /* Different conditional pattern */
        if (j % 3 == 0) {
            local_data[j] = alpha * j + results[j % N];
        } else if (j % 3 == 1) {
            local_data[j] = sqrtf(fabsf(results[j % N])) + j;
        } else {
            local_data[j] = cosf(j * 0.02f) * results[j % N];
        }
    }
    
    /* Copy back partial results */
    #pragma omp parallel for simd
    for (int k = 0; k < dynamic_count && k < N; k++) {
        data[k] += local_data[k];
    }
}

/* Host-side reference computation */
static void compute_reference(float* data, float* ref, int n, float alpha) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            ref[i] = data[i] * 2.0f;
        } else {
            ref[i] = data[i] / 2.0f;
        }
        ref[i] += sinf(i * 0.01f);
    }
}

int main() {
    float* data = (float*)malloc(N * sizeof(float));
    float* results = (float*)malloc(N * sizeof(float));
    float* reference = (float*)malloc(N * sizeof(float));
    
    if (!data || !results || !reference) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        data[i] = (float)i / 100.0f;
        results[i] = 0.0f;
        reference[i] = 0.0f;
    }
    
    /* Host-side OpenMP parallel region calling the offloading function */
    #pragma omp parallel num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        float alpha = 1.5f + thread_id * 0.1f;
        int dynamic_count = M + thread_id * 100;
        
        /* Each thread calls the offloading function with different parameters */
        process_on_gpu(data, results, dynamic_count, alpha);
    }
    
    /* Compute reference on host */
    compute_reference(data, reference, N, 1.5f);
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    
    #pragma omp parallel for reduction(+:errors)
    for (int i = 0; i < N; i++) {
        if (fabsf(results[i] - reference[i]) > tolerance) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All %d elements computed correctly on GPU\n", N);
        
        /* Additional verification - compute sum */
        float sum = 0.0f;
        #pragma omp parallel for reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += results[i];
        }
        printf("Sum of results: %f\n", sum);
    } else {
        printf("FAILURE: %d elements differ from reference\n", errors);
        
        /* Print first few mismatches */
        int max_errors = 5;
        for (int i = 0; i < N && max_errors > 0; i++) {
            if (fabsf(results[i] - reference[i]) > tolerance) {
                printf("Mismatch at index %d: GPU=%f, Host=%f\n", 
                       i, results[i], reference[i]);
                max_errors--;
            }
        }
    }
    
    free(data);
    free(results);
    free(reference);
    
    return 0;
}
