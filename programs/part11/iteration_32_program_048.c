/* test_omp_simt.c
 * This program is designed to trigger the SIMT transformation path in GCC's omp-low.cc,
 * specifically targeting the uncovered block that builds a conditional wrapper
 * around an OpenMP for-loop using IFN_GOMP_USE_SIMT.
 *
 * Compilation suggestions:
 *   gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower -c test_omp_simt.c
 *   gcc -O3 -fopenmp -fopenmp-simd -ftree-vectorize -fdump-tree-vect -c test_omp_simt.c
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
    double mixed_sum = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.1f;
        indices[i] = (i * 3) % SIZE;
        mixed_data[i] = (double)i * 0.05;
    }
    
    for (int i = 0; i < STRIDE_SIZE * 2; i++) {
        strided_data[i] = (float)i * 0.2f;
    }
    
    /* 
     * Conditional SIMD execution - may trigger IFN_GOMP_USE_SIMT 
     * This mimics runtime decision for SIMD execution
     */
    if (use_simd) {
        /* 
         * Target offloading directive with SIMD - often uses SIMT transformation
         * for GPU execution
         */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE], mixed_data[0:SIZE]) \
            map(to: indices[0:SIZE]) \
            reduction(+:sum, mixed_sum) \
            safelen(16)
        for (int i = 0; i < SIZE; i++) {
            /* Complex data-dependent computation with conditional */
            if (data[i] > 50.0f) {
                data[i] = sqrtf(data[i]);
            } else {
                data[i] = data[i] * 2.0f + 1.0f;
            }
            
            /* Non-contiguous memory access */
            float temp = data[indices[i]] * 0.5f;
            
            /* Mixed data type operations */
            mixed_data[i] = mixed_data[i] * 2.0 + (double)temp;
            
            /* Reduction with data-dependent condition */
            if (i % 8 == 0) {
                sum += data[i];
                mixed_sum += mixed_data[i];
            }
        }
    } else {
        /* Sequential fallback - same computation */
        for (int i = 0; i < SIZE; i++) {
            if (data[i] > 50.0f) {
                data[i] = sqrtf(data[i]);
            } else {
                data[i] = data[i] * 2.0f + 1.0f;
            }
            
            float temp = data[indices[i]] * 0.5f;
            mixed_data[i] = mixed_data[i] * 2.0 + (double)temp;
            
            if (i % 8 == 0) {
                sum += data[i];
                mixed_sum += mixed_data[i];
            }
        }
    }
    
    /* 
     * Unconditional SIMD loop with non-unit stride and safelen clause
     * This ensures SIMD constructs are always parsed
     */
    #pragma omp simd safelen(8) aligned(strided_data:32)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-unit stride access pattern */
        strided_data[i * 2] = strided_data[i * 2] * 3.0f + 
                             strided_data[i * 2 + 1] * 0.5f;
        
        /* Additional conditional to create complex control flow */
        if (strided_data[i * 2] > 100.0f) {
            strided_data[i * 2] = 100.0f;
        }
    }
    
    /* 
     * Nested loops with SIMD on inner loop
     * This creates additional opportunities for SIMT transformation
     */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < 32; i++) {
        #pragma omp simd linear(j:1) reduction(+:sum)
        for (int j = 0; j < 32; j++) {
            int idx = i * 32 + j;
            if (idx < SIZE) {
                data[idx] = data[idx] * 1.1f;
                sum += data[idx] * 0.01f;
            }
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results:\n");
    printf("  Sum: %.2f\n", sum);
    printf("  Mixed sum: %.2f\n", mixed_sum);
    printf("  Sample data[0]: %.2f\n", data[0]);
    printf("  Sample data[100]: %.2f\n", data[100]);
    printf("  Strided data[10]: %.2f\n", strided_data[20]);
    
    /* Additional runtime condition that might affect SIMD execution */
    int dynamic_condition = (sum > 1000.0f);
    
    /* Another conditional SIMD block */
    if (dynamic_condition) {
        #pragma omp simd
        for (int i = 0; i < SIZE; i += 4) {
            /* Vector-like operation on groups of 4 */
            data[i] = data[i] + data[i+1] + data[i+2] + data[i+3];
        }
    }
    
    return 0;
}
