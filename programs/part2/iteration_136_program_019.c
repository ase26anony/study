#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define CHUNK_SIZE 256

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    int n, float scale, int dynamic_count) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        device(0) num_teams((n+CHUNK_SIZE-1)/CHUNK_SIZE) thread_limit(256) \
        simdlen(32)
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
        device(0) num_teams(16) thread_limit(128) \
        simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* More complex conditional with math function */
        if (i % 3 == 0) {
            c[i] = c[i] * sinf((float)i * 0.01f);
        } else if (i % 3 == 1) {
            c[i] = c[i] * cosf((float)i * 0.01f);
        } else {
            c[i] = sqrtf(fabsf(c[i]));
        }
    }
}

/* Another helper with different loop structure */
static __attribute__((noinline))
void process_reduction(float* restrict data, int n, int dynamic_iters) {
    float sum = 0.0f;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: data[0:n]) map(tofrom: sum) reduction(+:sum) \
        device(0) num_teams(8) thread_limit(64) \
        simdlen(8)
    for (int i = 0; i < n; i++) {
        /* Nested conditionals to create more complex control flow */
        if (data[i] > 0.0f) {
            sum += data[i] * data[i];
        } else if (data[i] < -1.0f) {
            sum += -data[i];
        }
    }
    
    /* Loop with early exit simulation */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:dynamic_iters]) \
        device(0) num_teams(4) thread_limit(32) \
        simdlen(4)
    for (int i = 0; i < dynamic_iters; i++) {
        /* Complex conditional that might affect SIMT lane divergence */
        float val = data[i];
        if (val != 0.0f) {
            data[i] = 1.0f / val;
            if (data[i] > 100.0f) {
                data[i] = 100.0f;
            }
        }
    }
}

int main(int argc, char** argv) {
    float *a, *b, *c, *host_c, *data;
    int dynamic_count = (argc > 1) ? atoi(argv[1]) : 5000;
    
    if (dynamic_count > N) dynamic_count = N;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    host_c = (float*)malloc(N * sizeof(float));
    data = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(N - i) * 0.05f;
        c[i] = 0.0f;
        data[i] = sinf((float)i * 0.01f);
    }
    
    /* Host-side OpenMP parallel region calling GPU offloading function */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the GPU offloading function with different data slices */
        int chunk = N / 4;
        int start = tid * chunk;
        int end = (tid == 3) ? N : start + chunk;
        
        process_on_gpu(&a[start], &b[start], &c[start], 
                      end - start, 2.0f + tid * 0.1f, dynamic_count / 4);
    }
    
    /* Compute reference on host */
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            host_c[i] = a[i] * 2.0f + b[i];
        } else {
            host_c[i] = a[i] * (2.0f * 0.5f) + b[i] * 2.0f;
        }
    }
    
    /* Process second part on host for reference */
    for (int i = 0; i < dynamic_count; i++) {
        if (i % 3 == 0) {
            host_c[i] = host_c[i] * sinf((float)i * 0.01f);
        } else if (i % 3 == 1) {
            host_c[i] = host_c[i] * cosf((float)i * 0.01f);
        } else {
            host_c[i] = sqrtf(fabsf(host_c[i]));
        }
    }
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    for (int i = 0; i < N; i++) {
        if (i < dynamic_count) {
            if (fabsf(c[i] - host_c[i]) > tolerance) {
                errors++;
                if (errors < 10) {
                    printf("Mismatch at %d: device=%f host=%f\n", 
                           i, c[i], host_c[i]);
                }
            }
        }
    }
    
    /* Call reduction function from another parallel region */
    #pragma omp parallel
    {
        process_reduction(data, N, dynamic_count);
    }
    
    /* Final validation */
    if (errors == 0) {
        printf("SUCCESS: All GPU computations match host reference\n");
        
        /* Additional check on reduction results */
        float host_sum = 0.0f;
        for (int i = 0; i < N; i++) {
            if (data[i] > 0.0f) {
                host_sum += data[i] * data[i];
            } else if (data[i] < -1.0f) {
                host_sum += -data[i];
            }
        }
        printf("Reduction check completed\n");
    } else {
        printf("FAILURE: %d mismatches found\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(host_c);
    free(data);
    
    return errors == 0 ? 0 : 1;
}
