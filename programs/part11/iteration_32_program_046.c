/* test_omp_simt.c
 * Designed to trigger SIMT transformation in omp-low.cc lines 2941-2975
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower -c test_omp_simt.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define STRIDE_SIZE 512

int main(int argc, char *argv[]) {
    /* Runtime condition to control SIMD execution path */
    int use_simd = argc > 1;  /* SIMD if any argument provided */
    
    /* Arrays with mixed access patterns */
    float data[SIZE];
    double dbl_data[SIZE];
    int indices[SIZE];
    float result[SIZE/2];  /* For non-contiguous access */
    
    /* Reduction variable */
    float sum = 0.0f;
    double dbl_sum = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)(i % 100) * 0.1f;
        dbl_data[i] = (double)(i % 50) * 0.2;
        indices[i] = (i * 3) % SIZE;  /* Non-linear indexing */
    }
    
    /* ============================================
     * Conditional SIMD execution - may trigger IFN_GOMP_USE_SIMT
     * ============================================ */
    if (use_simd) {
        /* Target offloading with SIMD - likely to use SIMT transformation */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE], dbl_data[0:SIZE]) \
            map(to: indices[0:SIZE]) \
            reduction(+:sum, dbl_sum) \
            safelen(32)
        for (int i = 0; i < SIZE; i++) {
            /* Mixed data types and conditional inside loop */
            float temp = data[i] * 2.0f;
            if (temp > 50.0f) {
                temp = 50.0f;  /* Data-dependent condition */
            }
            data[i] = temp + 1.0f;
            
            /* Double precision computation */
            dbl_data[i] = dbl_data[i] * 1.5 + sin((double)i * 0.01);
            
            /* Reduction with complex expression */
            sum += data[i] * 0.01f;
            dbl_sum += dbl_data[i];
            
            /* Early exit condition - increases complexity */
            if (i > 900 && data[i] < 10.0f) {
                /* This may affect SIMT lane management */
                data[i] = data[i] * 0.5f;
            }
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < SIZE; i++) {
            float temp = data[i] * 2.0f;
            if (temp > 50.0f) {
                temp = 50.0f;
            }
            data[i] = temp + 1.0f;
            dbl_data[i] = dbl_data[i] * 1.5 + sin((double)i * 0.01);
            sum += data[i] * 0.01f;
            dbl_sum += dbl_data[i];
            if (i > 900 && data[i] < 10.0f) {
                data[i] = data[i] * 0.5f;
            }
        }
    }
    
    /* ============================================
     * Unconditional SIMD with non-contiguous access
     * Always present to ensure SIMD constructs are parsed
     * ============================================ */
    #pragma omp simd safelen(16) aligned(data:32) linear(i:1)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-unit stride access pattern */
        data[i*2] = data[i*2] * 3.0f + (float)(i % 4);
        
        /* Indirect indexing - complex memory pattern */
        int idx = indices[i];
        if (idx < SIZE) {
            dbl_data[idx] = dbl_data[idx] * 0.9;
        }
    }
    
    /* ============================================
     * Nested loops with inner SIMD
     * ============================================ */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            /* Inner SIMD loop with reduction */
            float local_sum = 0.0f;
            #pragma omp simd reduction(+:local_sum) safelen(8)
            for (int k = 0; k < 8; k++) {
                int index = i * 32 + j + k * 4;
                if (index < SIZE) {
                    local_sum += data[index] * (float)k;
                }
            }
            result[(i * 32 + j) % (SIZE/2)] = local_sum;
        }
    }
    
    /* ============================================
     * Vector types with OpenMP SIMD
     * ============================================ */
    typedef float v4sf __attribute__((vector_size(16)));
    v4sf vec_data[SIZE/4];
    
    /* Initialize vector array */
    for (int i = 0; i < SIZE/4; i++) {
        vec_data[i] = (v4sf){data[i*4], data[i*4+1], data[i*4+2], data[i*4+3]};
    }
    
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        /* Vector operations */
        v4sf multiplier = {1.1f, 1.2f, 1.3f, 1.4f};
        vec_data[i] = vec_data[i] * multiplier;
        
        /* Store back to scalar array */
        data[i*4] = vec_data[i][0];
        data[i*4+1] = vec_data[i][1];
        data[i*4+2] = vec_data[i][2];
        data[i*4+3] = vec_data[i][3];
    }
    
    /* Output to prevent dead code elimination */
    printf("SIMD mode: %s\n", use_simd ? "enabled" : "disabled");
    printf("Sum: %.2f, Double sum: %.2f\n", sum, dbl_sum);
    printf("Sample data[0]: %.2f, data[100]: %.2f, data[500]: %.2f\n", 
           data[0], data[100], data[500]);
    printf("Result[10]: %.2f, Result[100]: %.2f\n", result[10], result[100]);
    
    return 0;
}
