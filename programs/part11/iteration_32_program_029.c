/* test_omp_simt.c
 * 
 * This program is designed to trigger the SIMT transformation path in GCC's
 * omp-low.cc, specifically targeting the uncovered block that builds a
 * conditional wrapper around an OpenMP for-loop using IFN_GOMP_USE_SIMT.
 * 
 * Compilation recommendations:
 *   gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower -c test_omp_simt.c
 *   gcc -O3 -fopenmp -fopenmp-simd -ftree-vectorize -fdump-tree-vect -c test_omp_simt.c
 *   gcc -O2 -fopenmp -foffload=amdgcn-amdhsa -fdump-tree-omplower -c test_omp_simt.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define STRIDE_SIZE 512

int main(int argc, char *argv[]) {
    /* Use command-line argument to control SIMD execution path */
    int use_simd = (argc > 1);
    
    /* Arrays with different access patterns */
    float data[SIZE];
    float strided_data[STRIDE_SIZE * 2];
    int indices[SIZE];
    double mixed_data[SIZE];
    
    /* Reduction variable */
    float sum = 0.0f;
    double dsum = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.1f;
        indices[i] = (i * 3) % SIZE;  /* Non-linear indexing */
        mixed_data[i] = (double)i * 0.05;
    }
    
    for (int i = 0; i < STRIDE_SIZE * 2; i++) {
        strided_data[i] = (float)i * 0.2f;
    }
    
    /* 
     * UNCONDITIONAL SIMD LOOP with non-unit stride and safelen clause
     * This ensures SIMD constructs are always parsed
     */
    #pragma omp simd safelen(8) aligned(data:32)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-contiguous memory access with stride 2 */
        strided_data[i * 2] = strided_data[i * 2] * 3.0f + sinf((float)i);
    }
    
    /* 
     * CONDITIONAL SIMD EXECUTION BLOCK
     * The runtime condition may trigger the SIMT transformation with
     * IFN_GOMP_USE_SIMT internal function
     */
    if (use_simd) {
        printf("Using SIMD/offload path\n");
        
        /* 
         * TARGET OFFLOADING DIRECTIVE with SIMD
         * This is a prime candidate for SIMT transformation
         */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE], mixed_data[0:SIZE]) \
            map(to: indices[0:SIZE]) \
            reduction(+:sum, dsum) \
            private(i) \
            collapse(2)
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < 2; j++) {
                /* Complex data-dependent computation */
                float val = data[i] * 2.0f + (float)j;
                
                /* Conditional break inside SIMD loop - encourages SIMT */
                if (val > 100.0f && j == 1) {
                    /* This may create lane masking requirements */
                    val = 100.0f;
                }
                
                /* Mixed data type operations */
                double dval = mixed_data[i] * 0.5;
                dsum += dval;
                
                /* Indirect array access */
                int idx = indices[i];
                if (idx >= 0 && idx < SIZE) {
                    data[idx] = val + data[i];
                }
                
                sum += val;
            }
        }
        
        /* 
         * Nested loop with inner SIMD and reduction
         * Another candidate for SIMT transformation
         */
        float nested_sum = 0.0f;
        #pragma omp parallel for reduction(+:nested_sum)
        for (int i = 0; i < SIZE/2; i++) {
            #pragma omp simd linear(j:1) reduction(+:nested_sum)
            for (int j = 0; j < 4; j++) {
                /* Data-dependent condition inside SIMD loop */
                if (data[i * 2 + j % 2] > 50.0f) {
                    nested_sum += 1.0f;
                } else {
                    nested_sum += data[i * 2 + j % 2] * 0.1f;
                }
            }
        }
        sum += nested_sum;
        
    } else {
        /* Sequential fallback path */
        printf("Using sequential path\n");
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < 2; j++) {
                float val = data[i] * 2.0f + (float)j;
                if (val > 100.0f && j == 1) {
                    val = 100.0f;
                }
                double dval = mixed_data[i] * 0.5;
                dsum += dval;
                
                int idx = indices[i];
                if (idx >= 0 && idx < SIZE) {
                    data[idx] = val + data[i];
                }
                sum += val;
            }
        }
    }
    
    /* 
     * Additional SIMD loop with explicit vector types
     * Using GCC vector extensions to test SIMT handling
     */
    typedef float v4sf __attribute__((vector_size(16)));
    v4sf vector_data[SIZE/4];
    
    /* Initialize vector data */
    for (int i = 0; i < SIZE/4; i++) {
        for (int j = 0; j < 4; j++) {
            vector_data[i][j] = (float)(i * 4 + j) * 0.25f;
        }
    }
    
    /* SIMD loop operating on vector types */
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = vector_data[i] * 2.0f + 1.0f;
        
        /* Conditional operation on vector lanes */
        if (i % 3 == 0) {
            vector_data[i] = vector_data[i] * 0.5f;
        }
    }
    
    /* Prevent dead code elimination */
    printf("Results: sum = %f, dsum = %lf\n", sum, dsum);
    printf("Sample data[10] = %f, data[100] = %f\n", data[10], data[100]);
    printf("Sample strided_data[20] = %f\n", strided_data[20]);
    
    /* Use vector result */
    float vector_sum = 0.0f;
    for (int i = 0; i < SIZE/4; i++) {
        for (int j = 0; j < 4; j++) {
            vector_sum += vector_data[i][j];
        }
    }
    printf("Vector sum = %f\n", vector_sum);
    
    return 0;
}
