#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
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
        map(tofrom: c[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            c[i] = sqrtf(c[i]);
        } else if (i % 3 == 1) {
            c[i] = c[i] * c[i];
        } else {
            c[i] = 1.0f / (c[i] + 1.0f);
        }
    }
}

/* Host-side computation for verification */
static void compute_reference(float* a, float* b, float* ref, int n, float alpha) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            ref[i] = alpha * a[i] + b[i];
        } else {
            ref[i] = alpha * b[i] + a[i];
        }
    }
}

int main(int argc, char** argv) {
    float *a, *b, *c, *ref;
    float alpha = 2.5f;
    int dynamic_count = M;
    
    if (argc > 1) {
        dynamic_count = atoi(argv[1]);
        if (dynamic_count > N) dynamic_count = N;
    }
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    ref = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i / N;
        b[i] = (float)(N - i) / N;
        c[i] = 0.0f;
    }
    
    /* Host-side OpenMP parallel region wrapping the GPU call */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the GPU function with different data sections */
        #pragma omp for schedule(static)
        for (int t = 0; t < 4; t++) {
            int start = t * (N / 4);
            int end = (t == 3) ? N : (t + 1) * (N / 4);
            
            /* Call the GPU processing function */
            process_on_gpu(&a[start], &b[start], &c[start], 
                          end - start, alpha, dynamic_count / 4);
        }
    }
    
    /* Compute reference on host */
    compute_reference(a, b, ref, N, alpha);
    
    /* Apply second transformation on host for verification */
    for (int i = 0; i < dynamic_count; i++) {
        if (i % 3 == 0) {
            ref[i] = sqrtf(ref[i]);
        } else if (i % 3 == 1) {
            ref[i] = ref[i] * ref[i];
        } else {
            ref[i] = 1.0f / (ref[i] + 1.0f);
        }
    }
    
    /* Verify results */
    int errors = 0;
    float tolerance = 1e-4f;
    
    for (int i = 0; i < dynamic_count; i++) {
        if (fabsf(c[i] - ref[i]) > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Mismatch at index %d: GPU=%f, CPU=%f\n", 
                       i, c[i], ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All %d elements computed correctly on GPU\n", dynamic_count);
    } else {
        printf("FAILURE: %d errors found\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(ref);
    
    return errors > 0 ? 1 : 0;
}
