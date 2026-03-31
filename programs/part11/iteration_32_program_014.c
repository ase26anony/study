/* test_omp_simt.c
 * 
 * This program is designed to trigger the SIMT transformation path in GCC's
 * omp-low.cc, specifically targeting the uncovered block that builds a
 * conditional wrapper around an OpenMP for-loop using IFN_GOMP_USE_SIMT.
 * 
 * Compilation suggestions:
 * 1. For GPU offloading (NVPTX): gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower -c test_omp_simt.c
 * 2. For SIMD vectorization: gcc -O3 -fopenmp -fopenmp-simd -ftree-vectorize -fdump-tree-omplower-details -c test_omp_simt.c
 * 3. For AMD GPU offloading: gcc -O2 -fopenmp -foffload=amdgcn-amdhsa -fdump-tree-omplower -c test_omp_simt.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define STRIDE_SIZE 512

/* Mixed data types to encourage complex SIMT handling */
typedef float v4sf __attribute__((vector_size(16)));

int main(int argc, char *argv[]) {
    /* Runtime condition to potentially enable SIMD/SIMT path */
    int use_simd = argc > 1;  /* Use SIMD if any command-line argument given */
    
    /* Arrays with different access patterns */
    float data[SIZE];
    float strided_data[STRIDE_SIZE * 2];  /* For non-unit stride access */
    int indices[SIZE];
    double double_data[SIZE];  /* Mixed precision */
    v4sf vector_data[SIZE/4];  /* Explicit vector type */
    
    /* Reduction variable */
    float sum = 0.0f;
    double double_sum = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.1f;
        double_data[i] = (double)i * 0.05;
        indices[i] = (i * 7) % SIZE;  /* Non-linear indexing */
    }
    
    for (int i = 0; i < STRIDE_SIZE * 2; i++) {
        strided_data[i] = (float)i * 0.2f;
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = (v4sf){i*0.1f, i*0.2f, i*0.3f, i*0.4f};
    }
    
    /* 
     * UNCONDITIONAL SIMD LOOP with non-unit stride and safelen clause
     * This ensures SIMD constructs are always parsed and may trigger
     * the SIMT transformation path during lowering.
     */
    #pragma omp simd safelen(8)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-contiguous memory access pattern */
        strided_data[i*2] = strided_data[i*2] * 3.0f + sinf((float)i * 0.01f);
    }
    
    /* 
     * CONDITIONAL BLOCK: Runtime decision for SIMD/SIMT execution
     * This may trigger the conditional wrapper generation with IFN_GOMP_USE_SIMT
     */
    if (use_simd) {
        printf("Using SIMD/SIMT execution path\n");
        
        /* 
         * TARGET OFFLOADING DIRECTIVE with SIMD
         * This is a prime candidate for SIMT transformation, especially
         * with the reduction clause and complex loop body.
         */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE], double_data[0:SIZE]) \
            map(to: indices[0:SIZE]) \
            reduction(+:sum, double_sum) \
            private(indices)  /* Add complexity with private clause */
        for (int i = 0; i < SIZE; i++) {
            /* Data-dependent condition inside SIMD loop */
            if (data[i] > 50.0f) {
                /* Early exit-like behavior - challenging for SIMD/SIMT */
                data[i] = 50.0f;
            }
            
            /* Mixed data type operations */
            data[i] = data[i] * 2.0f + 1.0f;
            double_data[i] = double_data[i] * 1.5 + 0.5;
            
            /* Indirect indexing - complex memory access pattern */
            sum += data[indices[i]] * 0.1f;
            double_sum += double_data[i] * 0.01;
            
            /* Additional conditional that may require lane masking */
            if (i % 3 == 0) {
                data[i] *= 0.9f;
            }
        }
        
        /* 
         * NESTED LOOPS with SIMD on inner loop
         * Another pattern that may trigger SIMT transformation
         */
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                #pragma omp simd linear(k:1) aligned(data:16)
                for (int k = 0; k < 8; k++) {
                    int idx = i * 32 + j + k;
                    if (idx < SIZE) {
                        data[idx] = data[idx] * (float)(i + j + k) * 0.01f;
                    }
                }
            }
        }
        
    } else {
        printf("Using sequential execution path\n");
        
        /* Sequential version of the computation */
        for (int i = 0; i < SIZE; i++) {
            if (data[i] > 50.0f) {
                data[i] = 50.0f;
            }
            data[i] = data[i] * 2.0f + 1.0f;
            double_data[i] = double_data[i] * 1.5 + 0.5;
            sum += data[indices[i]] * 0.1f;
            double_sum += double_data[i] * 0.01;
            if (i % 3 == 0) {
                data[i] *= 0.9f;
            }
        }
    }
    
    /* 
     * Additional SIMD loop with explicit vector types
     * Using OpenMP 4.0's declare simd directive style
     */
    #pragma omp declare simd uniform(scale) linear(i:1)
    for (int i = 0; i < SIZE/4; i++) {
        float scale = (use_simd) ? 1.5f : 1.0f;
        vector_data[i] = vector_data[i] * scale + (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
    }
    
    /* Prevent dead code elimination by printing results */
    printf("Results:\n");
    printf("  Sum: %f\n", sum);
    printf("  Double Sum: %lf\n", double_sum);
    printf("  Sample data[0]: %f\n", data[0]);
    printf("  Sample data[100]: %f\n", data[100]);
    printf("  Sample strided_data[10]: %f\n", strided_data[10]);
    
    /* Verify results aren't NaN */
    if (isnan(sum) || isnan(double_sum)) {
        printf("ERROR: Invalid results detected\n");
        return 1;
    }
    
    return 0;
}
