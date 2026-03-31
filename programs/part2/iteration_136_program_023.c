/* test_simt_transformation.c
 * 
 * This program is designed to trigger the SIMT transformation in GCC's omp-low.cc
 * (lines 2941-2975) by using OpenMP target offloading with specific constructs.
 * It creates a scenario where the compiler must transform regular loops into
 * SIMT variants for GPU execution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function containing the target offloading regions.
 * The 'static' attribute prevents inlining, ensuring the transformation
 * pass processes the OpenMP constructs within this function.
 */
static void __attribute__((noinline)) 
process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
               float* restrict d, int dynamic_count, float scale) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = a[i] * scale + b[i];
        } else {
            c[i] = a[i] * 2.0f + b[i] * 0.5f;
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: d[0:dynamic_count]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_count; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            d[i] = d[i] * 2.0f + 1.0f;
        } else if (i % 3 == 1) {
            d[i] = d[i] * 0.5f - 1.0f;
        } else {
            d[i] = d[i] * 1.5f;
        }
    }
}

/* Host-side reference computation for validation */
static void compute_reference(float* a, float* b, float* ref_c, 
                              float* ref_d, int dynamic_count, float scale) {
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            ref_c[i] = a[i] * scale + b[i];
        } else {
            ref_c[i] = a[i] * 2.0f + b[i] * 0.5f;
        }
    }
    
    for (int i = 0; i < dynamic_count; i++) {
        if (i % 3 == 0) {
            ref_d[i] = ref_d[i] * 2.0f + 1.0f;
        } else if (i % 3 == 1) {
            ref_d[i] = ref_d[i] * 0.5f - 1.0f;
        } else {
            ref_d[i] = ref_d[i] * 1.5f;
        }
    }
}

int main(int argc, char* argv[]) {
    float *a, *b, *c, *d;
    float *ref_c, *ref_d;
    int dynamic_count = M;
    float scale = 3.14f;
    
    if (argc > 1) {
        dynamic_count = atoi(argv[1]);
        if (dynamic_count <= 0 || dynamic_count > N) {
            dynamic_count = M;
        }
    }
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(N * sizeof(float));
    ref_c = (float*)malloc(N * sizeof(float));
    ref_d = (float*)malloc(N * sizeof(float));
    
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(N - i) * 0.05f;
        c[i] = 0.0f;
        ref_c[i] = 0.0f;
        d[i] = (float)i * 0.2f;
        ref_d[i] = d[i];
    }
    
    /* Host-side reference computation */
    compute_reference(a, b, ref_c, ref_d, dynamic_count, scale);
    
    /* Reset device arrays */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        c[i] = 0.0f;
    }
    
    /* Nested parallelism: host parallel region calling target offloading */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the offloading function with different data slices */
        int chunk = dynamic_count / 4;
        int start = tid * chunk;
        int end = (tid == 3) ? dynamic_count : start + chunk;
        
        /* Adjust pointers for each thread's slice */
        float* d_slice = d + start;
        int slice_count = end - start;
        
        if (slice_count > 0) {
            /* Call the offloading function - this should trigger the SIMT transformation */
            process_on_gpu(a + start, b + start, c + start, 
                          d_slice, slice_count, scale + (float)tid * 0.1f);
        }
    }
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-5f;
    
    for (int i = 0; i < N; i++) {
        if (i < dynamic_count) {
            if (fabs(c[i] - ref_c[i]) > tolerance) {
                if (errors < 10) {
                    printf("Mismatch at c[%d]: device=%f, host=%f\n", 
                           i, c[i], ref_c[i]);
                }
                errors++;
            }
            if (fabs(d[i] - ref_d[i]) > tolerance) {
                if (errors < 10) {
                    printf("Mismatch at d[%d]: device=%f, host=%f\n", 
                           i, d[i], ref_d[i]);
                }
                errors++;
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All results match within tolerance %e\n", tolerance);
    } else {
        printf("FAILURE: %d errors found\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(ref_c);
    free(ref_d);
    
    return errors == 0 ? 0 : 1;
}
