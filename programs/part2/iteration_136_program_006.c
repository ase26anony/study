/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000
#define VALIDATE_TOL 1e-6

/* Non-inlineable helper function containing target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    int n, float alpha, int dynamic_count) {
    /* First target region with compile-time constant iteration count */
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
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_count]) map(tofrom: b[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            b[i] = sqrtf(fabsf(a[i])) + c[i % n];
        } else if (i % 3 == 1) {
            b[i] = sinf(a[i]) * cosf(c[i % n]);
        } else {
            b[i] = a[i] * c[i % n] - alpha;
        }
    }
}

/* Host-side reference computation */
static void compute_reference(float* a, float* b, float* c_ref, 
                              int n, float alpha, int dynamic_count) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + b[i];
        } else {
            c_ref[i] = alpha * b[i] + a[i];
        }
    }
    
    float* b_ref = (float*)malloc(dynamic_count * sizeof(float));
    for (int i = 0; i < dynamic_count; i++) {
        b_ref[i] = b[i];
    }
    
    for (int i = 0; i < dynamic_count; i++) {
        if (i % 3 == 0) {
            b_ref[i] = sqrtf(fabsf(a[i])) + c_ref[i % n];
        } else if (i % 3 == 1) {
            b_ref[i] = sinf(a[i]) * cosf(c_ref[i % n]);
        } else {
            b_ref[i] = a[i] * c_ref[i % n] - alpha;
        }
    }
    
    /* Copy back for validation */
    for (int i = 0; i < dynamic_count; i++) {
        b[i] = b_ref[i];
    }
    free(b_ref);
}

/* Validation function */
static int validate_results(float* c, float* c_ref, float* b, float* b_ref,
                           int n, int dynamic_count) {
    int errors = 0;
    
    for (int i = 0; i < n; i++) {
        if (fabsf(c[i] - c_ref[i]) > VALIDATE_TOL) {
            if (errors < 5) {
                printf("Mismatch at c[%d]: device=%f, host=%f\n", 
                       i, c[i], c_ref[i]);
            }
            errors++;
        }
    }
    
    for (int i = 0; i < dynamic_count; i++) {
        if (fabsf(b[i] - b_ref[i]) > VALIDATE_TOL) {
            if (errors < 10) {
                printf("Mismatch at b[%d]: device=%f, host=%f\n", 
                       i, b[i], b_ref[i]);
            }
            errors++;
        }
    }
    
    return errors;
}

int main(int argc, char** argv) {
    float *a, *b, *c, *c_ref, *b_ref;
    int dynamic_count = M;
    float alpha = 2.5f;
    
    if (argc > 1) {
        dynamic_count = atoi(argv[1]);
        if (dynamic_count <= 0 || dynamic_count > N*2) {
            dynamic_count = M;
        }
    }
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    b_ref = (float*)malloc(dynamic_count * sizeof(float));
    
    srand(42);
    for (int i = 0; i < N; i++) {
        a[i] = (float)rand() / RAND_MAX * 10.0f;
        b[i] = (float)rand() / RAND_MAX * 5.0f;
    }
    
    /* Save original b values for reference computation */
    for (int i = 0; i < dynamic_count; i++) {
        b_ref[i] = b[i];
    }
    
    /* Host-side OpenMP parallel region calling the offloading function */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        if (tid == 0) {
            /* First thread calls GPU processing */
            process_on_gpu(a, b, c, N, alpha, dynamic_count);
        } else {
            /* Second thread does nothing or could do host computation */
            #pragma omp barrier
        }
    }
    
    /* Compute reference on host */
    compute_reference(a, b_ref, c_ref, N, alpha, dynamic_count);
    
    /* Validate results */
    int errors = validate_results(c, c_ref, b, b_ref, N, dynamic_count);
    
    if (errors == 0) {
        printf("SUCCESS: All results match within tolerance %e\n", VALIDATE_TOL);
    } else {
        printf("FAILURE: %d errors found\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(c_ref);
    free(b_ref);
    
    return errors > 0 ? 1 : 0;
}
