/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000
#define VALIDATE_EPS 1e-6

/* Non-inlineable helper function containing target offloading */
static void __attribute__((noinline)) 
process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
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
        map(to: a[0:dynamic_count]) map(tofrom: c[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* More complex conditional with math function */
        if (i % 3 == 0) {
            c[i] = c[i] * sinf((float)i / 100.0f);
        } else if (i % 3 == 1) {
            c[i] = c[i] + cosf((float)i / 50.0f);
        } else {
            c[i] = sqrtf(fabsf(c[i]));
        }
    }
}

/* Host-side reference computation */
static void compute_reference(float* a, float* b, float* ref, 
                              int n, float alpha, int dynamic_count) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            ref[i] = alpha * a[i] + b[i];
        } else {
            ref[i] = alpha * b[i] + a[i];
        }
    }
    
    for (int i = 0; i < dynamic_count; i++) {
        if (i % 3 == 0) {
            ref[i] = ref[i] * sinf((float)i / 100.0f);
        } else if (i % 3 == 1) {
            ref[i] = ref[i] + cosf((float)i / 50.0f);
        } else {
            ref[i] = sqrtf(fabsf(ref[i]));
        }
    }
}

/* Validation function */
static int validate_results(float* ref, float* res, int n) {
    for (int i = 0; i < n; i++) {
        if (fabsf(ref[i] - res[i]) > VALIDATE_EPS) {
            printf("Mismatch at index %d: ref=%f, res=%f\n", 
                   i, ref[i], res[i]);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv) {
    float *a, *b, *c, *ref;
    int dynamic_count = M;
    
    if (argc > 1) {
        dynamic_count = atoi(argv[1]);
        dynamic_count = (dynamic_count < 1) ? M : dynamic_count;
        dynamic_count = (dynamic_count > N) ? N : dynamic_count;
    }
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    ref = (float*)malloc(N * sizeof(float));
    
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a[i] = (float)i / 100.0f;
        b[i] = (float)(N - i) / 100.0f;
        c[i] = 0.0f;
        ref[i] = 0.0f;
    }
    
    /* Host-side OpenMP parallel region wrapping the GPU call */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the GPU processing function */
        #pragma omp master
        {
            printf("Starting GPU offloading from %d threads\n", 
                   omp_get_num_threads());
        }
        
        /* Only thread 0 and 1 perform actual computation to create variation */
        if (tid < 2) {
            float local_alpha = 1.5f + (float)tid * 0.1f;
            process_on_gpu(a, b, c, N, local_alpha, dynamic_count);
        }
    }
    
    /* Compute reference on host */
    compute_reference(a, b, ref, N, 1.5f, dynamic_count);
    
    /* Validate results */
    if (validate_results(ref, c, dynamic_count)) {
        printf("SUCCESS: GPU computation matches reference\n");
        
        /* Print some sample values */
        printf("Sample values (indices 0, %d, %d):\n", 
               dynamic_count/2, dynamic_count-1);
        printf("  c[0] = %f (ref = %f)\n", c[0], ref[0]);
        printf("  c[%d] = %f (ref = %f)\n", 
               dynamic_count/2, c[dynamic_count/2], ref[dynamic_count/2]);
        printf("  c[%d] = %f (ref = %f)\n", 
               dynamic_count-1, c[dynamic_count-1], ref[dynamic_count-1]);
    } else {
        printf("FAILURE: Results don't match\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(ref);
    
    return 0;
}
