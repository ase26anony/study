/* Test program to trigger SIMT transformation in GCC's omp-low.cc
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -foffload="-O2" -o test_simt test_simt.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define N 10000
#define M 5000
#define VALIDATION_TOL 1e-6

/* Non-inlineable helper function containing target offloading regions */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    int n, float alpha, int dynamic_count) {
    /* First target region: compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < n; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = alpha * a[i] + b[i];
        } else {
            c[i] = alpha * b[i] + a[i];
        }
    }
    
    /* Second target region: dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_count]) map(tofrom: b[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* More complex conditional logic */
        if (i % 3 == 0) {
            b[i] = c[i] * 2.0f - a[i];
        } else if (i % 3 == 1) {
            b[i] = sqrtf(fabsf(c[i] + a[i]));
        } else {
            b[i] = sinf(c[i]) * cosf(a[i]);
        }
    }
    
    /* Third loop: reduction pattern */
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
        map(to: c[0:n]) map(tofrom: sum) reduction(+:sum) \
        device(0) num_teams(16) thread_limit(64) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += c[i] * (i % 10);
    }
    
    /* Use the result to prevent dead code elimination */
    c[0] += sum / n;
}

/* Host-side reference computation for validation */
static void compute_reference(float* restrict a, float* restrict b, 
                              float* restrict c_ref, int n, float alpha) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + b[i];
        } else {
            c_ref[i] = alpha * b[i] + a[i];
        }
    }
}

int main() {
    float *a, *b, *c, *c_ref;
    int dynamic_count = M;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c || !c_ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 50) % 100) * 0.2f;
        c[i] = 0.0f;
        c_ref[i] = 0.0f;
    }
    
    /* Host-side OpenMP parallel region wrapping the GPU call */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the GPU function with different data sections */
        #pragma omp for schedule(static)
        for (int chunk = 0; chunk < 4; chunk++) {
            int start = chunk * (N / 4);
            int end = (chunk == 3) ? N : start + (N / 4);
            
            /* Call GPU processing function - nested parallelism */
            process_on_gpu(&a[start], &b[start], &c[start], 
                          end - start, 1.5f, dynamic_count / 4);
        }
    }
    
    /* Compute reference on host */
    compute_reference(a, b, c_ref, N, 1.5f);
    
    /* Validate results */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > VALIDATION_TOL) {
            errors++;
            if (errors <= 10) {
                printf("Mismatch at index %d: device=%f, host=%f\n", 
                       i, c[i], c_ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All results match within tolerance\n");
    } else {
        printf("FAILURE: %d mismatches found\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(c_ref);
    
    return errors > 0 ? 1 : 0;
}
