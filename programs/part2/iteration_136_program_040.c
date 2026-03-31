/* Test program to trigger SIMT transformation in GCC's omp-low.cc
 * Specifically targets lines 2941-2975 in the SIMT transformation pass
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -foffload="-O2" -o test_simt test_simt.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function containing target offloading regions */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int dynamic_n, float alpha) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body to influence SIMT transformation */
        if (i % 2 == 0) {
            c[i] = a[i] * alpha + b[i];
        } else {
            c[i] = a[i] * 2.0f + b[i] * 0.5f;
        }
        
        /* Additional computation to prevent loop optimization */
        c[i] += sinf(i * 0.01f) * 0.1f;
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: d[0:dynamic_n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int j = 0; j < dynamic_n; j++) {
        /* Different conditional pattern */
        if (j % 3 == 0) {
            d[j] = d[j] * 1.5f + j * 0.01f;
        } else if (j % 3 == 1) {
            d[j] = d[j] * 0.75f - j * 0.005f;
        } else {
            d[j] = sqrtf(fabsf(d[j])) + j * 0.001f;
        }
    }
    
    /* Third loop with reduction to add complexity */
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
        map(to: c[0:N]) map(tofrom: sum) reduction(+:sum) \
        device(0) num_teams(16) thread_limit(64) simdlen(8)
    for (int k = 0; k < N; k += 2) {
        sum += c[k] * c[k+1];
    }
    
    /* Use the result to prevent dead code elimination */
    d[0] += sum * 0.0001f;
}

/* Host-side reference computation for validation */
static void compute_reference(float* a, float* b, float* c_ref, 
                              float* d_ref, int dynamic_n, float alpha) {
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            c_ref[i] = a[i] * alpha + b[i];
        } else {
            c_ref[i] = a[i] * 2.0f + b[i] * 0.5f;
        }
        c_ref[i] += sinf(i * 0.01f) * 0.1f;
    }
    
    for (int j = 0; j < dynamic_n; j++) {
        if (j % 3 == 0) {
            d_ref[j] = d_ref[j] * 1.5f + j * 0.01f;
        } else if (j % 3 == 1) {
            d_ref[j] = d_ref[j] * 0.75f - j * 0.005f;
        } else {
            d_ref[j] = sqrtf(fabsf(d_ref[j])) + j * 0.001f;
        }
    }
    
    float sum = 0.0f;
    for (int k = 0; k < N; k += 2) {
        sum += c_ref[k] * c_ref[k+1];
    }
    d_ref[0] += sum * 0.0001f;
}

int main() {
    float *a, *b, *c, *d;
    float *c_ref, *d_ref;
    int dynamic_n = M;
    float alpha = 1.5f;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    d_ref = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i / N;
        b[i] = (float)(N - i) / N;
        c[i] = 0.0f;
        d[i] = (float)i * 0.1f;
        c_ref[i] = 0.0f;
        d_ref[i] = (float)i * 0.1f;
    }
    
    /* Host-side OpenMP parallel region wrapping the GPU call */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the GPU processing function */
        #pragma omp barrier
        
        if (tid == 0) {
            printf("Starting GPU offloading from thread %d\n", tid);
            
            /* Call the function containing target regions */
            process_on_gpu(a, b, c, d, dynamic_n, alpha);
            
            /* Compute reference on host */
            compute_reference(a, b, c_ref, d_ref, dynamic_n, alpha);
            
            /* Validate results */
            int errors = 0;
            float tolerance = 1e-4f;
            
            for (int i = 0; i < N; i++) {
                if (fabsf(c[i] - c_ref[i]) > tolerance) {
                    errors++;
                    if (errors < 5) {
                        printf("Mismatch at c[%d]: GPU=%f, Host=%f\n", 
                               i, c[i], c_ref[i]);
                    }
                }
            }
            
            for (int i = 0; i < dynamic_n; i++) {
                if (fabsf(d[i] - d_ref[i]) > tolerance) {
                    errors++;
                    if (errors < 5) {
                        printf("Mismatch at d[%d]: GPU=%f, Host=%f\n", 
                               i, d[i], d_ref[i]);
                    }
                }
            }
            
            if (errors == 0) {
                printf("SUCCESS: All GPU computations match host reference\n");
            } else {
                printf("FAILURE: %d mismatches found\n", errors);
            }
        }
        
        #pragma omp barrier
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(c_ref);
    free(d_ref);
    
    return 0;
}
