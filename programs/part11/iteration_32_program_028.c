/* test_omp_simt.c
 * Designed to trigger SIMT transformation in omp-low.cc lines 2941-2975
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower -c test_omp_simt.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define STRIDE 2

/* Mixed data types and vector operations */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

int main(int argc, char *argv[]) {
    /* Runtime condition for conditional SIMD execution */
    int use_simd = argc > 1;  /* Enable SIMD path if any argument given */
    
    /* Arrays with different access patterns */
    float data[N];
    double dbl_data[N];
    int indices[N];
    float result[N/STRIDE];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        data[i] = (float)(i % 100) * 0.1f;
        dbl_data[i] = (double)(i % 50) * 0.2;
        indices[i] = (i * 7) % N;  /* Non-linear pattern */
    }
    
    float sum = 0.0f;
    double dbl_sum = 0.0;
    
    /* UNCONDITIONAL SIMD loop with complex patterns - always present */
    /* This ensures SIMD constructs are parsed regardless of runtime condition */
    #pragma omp simd safelen(16) aligned(data:16) linear(i:1)
    for (int i = 0; i < N/STRIDE; i++) {
        /* Non-contiguous memory access with stride */
        data[i*STRIDE] = data[i*STRIDE] * 3.0f + sinf((float)i * 0.01f);
        
        /* Indirect indexing */
        result[i] = data[indices[i]] * 2.0f;
        
        /* Mixed type operation */
        dbl_data[i] = (double)data[i*STRIDE] * 0.5;
    }
    
    /* CONDITIONAL SIMD execution path - may trigger IFN_GOMP_USE_SIMT */
    if (use_simd) {
        printf("Using SIMD/offload path\n");
        
        /* Target offloading directive with SIMD - likely to trigger SIMT transformation */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:N/2]) map(to: indices[0:N/2]) \
            reduction(+:sum) collapse(2) num_teams(4) thread_limit(128)
        for (int i = 0; i < N/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                
                /* Data-dependent condition inside SIMD loop */
                if (data[idx] > 50.0f) {
                    /* Early exit - creates complex control flow */
                    data[idx] = 50.0f;
                }
                
                /* Reduction operation */
                sum += data[idx] * 0.1f;
                
                /* Complex computation with mixed operations */
                data[idx] = data[idx] * 2.0f + 1.0f;
                
                /* Vector type operations within SIMD loop */
                v4sf vec_data = {data[idx], data[idx]*0.5f, 
                                 data[idx]*0.25f, data[idx]*0.125f};
                float vec_sum = vec_data[0] + vec_data[1] + vec_data[2] + vec_data[3];
                data[idx] += vec_sum * 0.01f;
            }
        }
        
        /* Nested SIMD with linear clause */
        #pragma omp simd linear(i:1) reduction(+:dbl_sum)
        for (int i = 0; i < N; i += 4) {
            /* Process 4 elements at a time mimicking vectorization */
            for (int k = 0; k < 4 && (i+k) < N; k++) {
                dbl_data[i+k] = dbl_data[i+k] * 1.5 + (double)k * 0.1;
                dbl_sum += dbl_data[i+k];
            }
        }
    } else {
        /* Sequential fallback - same computation without SIMD */
        printf("Using sequential path\n");
        for (int i = 0; i < N/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (data[idx] > 50.0f) {
                    data[idx] = 50.0f;
                }
                sum += data[idx] * 0.1f;
                data[idx] = data[idx] * 2.0f + 1.0f;
            }
        }
        
        for (int i = 0; i < N; i++) {
            dbl_data[i] = dbl_data[i] * 1.5 + (double)(i % 4) * 0.1;
            dbl_sum += dbl_data[i];
        }
    }
    
    /* Additional SIMD construct with conditional execution inside */
    /* This may create the conditional wrapper structure */
    int threshold = N/4;
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        /* Conditional store - creates lane masking requirements */
        if (i > threshold) {
            data[i] = data[i] * 0.5f;
        } else {
            data[i] = data[i] * 2.0f;
        }
        
        /* Cross-lane dependency simulation */
        if (i > 0) {
            data[i] += data[i-1] * 0.01f;
        }
    }
    
    /* Prevent dead code elimination */
    printf("Results: sum = %.2f, dbl_sum = %.2f\n", sum, dbl_sum);
    printf("Sample data[0], data[100], data[500]: %.2f, %.2f, %.2f\n", 
           data[0], data[100], data[500]);
    printf("Sample result[10]: %.2f\n", result[10]);
    
    return 0;
}
