#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* data, float* results, int dynamic_count, float a) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: data[0:N]) map(from: results[0:N]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Complex loop body with conditional to influence SIMT transformation */
        if (i % 2 == 0) {
            results[i] = data[i] * a + sinf((float)i * 0.01f);
        } else {
            results[i] = data[i] * a - cosf((float)i * 0.01f);
        }
        
        /* Additional computation to prevent optimization */
        if (results[i] > 100.0f) {
            results[i] = 100.0f;
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: results[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* Different computation pattern */
        float temp = results[i];
        for (int j = 0; j < 3; j++) {  /* Small inner loop */
            temp = temp * 0.9f + (float)j * 0.1f;
        }
        
        /* Conditional with data-dependent branch */
        if (temp > 50.0f && i % 3 == 0) {
            results[i] = sqrtf(temp);
        } else {
            results[i] = logf(fabsf(temp) + 1.0f);
        }
    }
}

/* Another helper with different loop structure */
static __attribute__((noinline))
void process_partial_gpu(float* data, float* results, int start, int end) {
    #pragma omp target teams distribute parallel for simd \
        map(to: data[start:end-start]) map(tofrom: results[start:end-start]) \
        device(0) num_teams(16) thread_limit(64) simdlen(8)
    for (int i = start; i < end; i++) {
        /* Reduction-like pattern */
        float sum = 0.0f;
        for (int k = 0; k < 4; k++) {
            sum += data[i] * (float)k;
        }
        results[i] = sum / 4.0f;
        
        /* Nested conditional */
        if (i % 4 == 0) {
            results[i] += 1.0f;
        } else if (i % 4 == 1) {
            results[i] += 2.0f;
        }
    }
}

int main() {
    float* data = (float*)malloc(N * sizeof(float));
    float* results = (float*)malloc(N * sizeof(float));
    float* host_ref = (float*)malloc(N * sizeof(float));
    
    if (!data || !results || !host_ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        data[i] = (float)i * 0.1f;
        results[i] = 0.0f;
        host_ref[i] = 0.0f;
    }
    
    /* Host-side OpenMP parallel region calling GPU offloading function */
    #pragma omp parallel num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        
        /* Different threads process different parts */
        if (thread_id == 0) {
            /* Call GPU processing function from within OpenMP parallel region */
            process_on_gpu(data, results, M, 2.5f);
        }
        
        #pragma omp barrier
        
        if (thread_id == 1) {
            /* Process another part on GPU */
            process_partial_gpu(data, results, 0, N/2);
        }
        
        if (thread_id == 2) {
            /* Process remaining part on GPU */
            process_partial_gpu(data, results, N/2, N);
        }
    }
    
    /* Compute reference on host */
    float a = 2.5f;
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        if (i < M) {
            if (i % 2 == 0) {
                host_ref[i] = data[i] * a + sinf((float)i * 0.01f);
            } else {
                host_ref[i] = data[i] * a - cosf((float)i * 0.01f);
            }
            
            if (host_ref[i] > 100.0f) {
                host_ref[i] = 100.0f;
            }
            
            /* Second computation phase */
            float temp = host_ref[i];
            for (int j = 0; j < 3; j++) {
                temp = temp * 0.9f + (float)j * 0.1f;
            }
            
            if (temp > 50.0f && i % 3 == 0) {
                host_ref[i] = sqrtf(temp);
            } else {
                host_ref[i] = logf(fabsf(temp) + 1.0f);
            }
        } else {
            /* For i >= M, only apply partial processing */
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                sum += data[i] * (float)k;
            }
            host_ref[i] = sum / 4.0f;
            
            if (i % 4 == 0) {
                host_ref[i] += 1.0f;
            } else if (i % 4 == 1) {
                host_ref[i] += 2.0f;
            }
        }
    }
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    
    #pragma omp parallel for reduction(+:errors)
    for (int i = 0; i < N; i++) {
        if (fabsf(results[i] - host_ref[i]) > tolerance) {
            #pragma omp atomic
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All %d elements computed correctly on GPU\n", N);
    } else {
        printf("FAILURE: %d elements differ between GPU and host\n", errors);
    }
    
    /* Cleanup */
    free(data);
    free(results);
    free(host_ref);
    
    return errors > 0 ? 1 : 0;
}
