/* test_omp_simt.c - Program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Mixed data types and vector type for complex patterns */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

#define SIZE 1024
#define STRIDE_SIZE 512

int main(int argc, char *argv[]) {
    /* Runtime condition for conditional SIMD execution */
    int use_simd = (argc > 1);
    int use_offload = (argc > 2);
    
    /* Arrays with different access patterns */
    float data[SIZE];
    double dbl_data[SIZE];
    int indices[SIZE];
    float strided_data[STRIDE_SIZE * 2]; /* For non-unit stride access */
    
    /* Reduction variables */
    float sum = 0.0f;
    double dbl_sum = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.5f;
        dbl_data[i] = (double)i * 0.25;
        indices[i] = (i * 3) % SIZE; /* Non-linear indexing */
    }
    
    for (int i = 0; i < STRIDE_SIZE * 2; i++) {
        strided_data[i] = (float)i * 0.1f;
    }
    
    /* Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT */
    if (use_simd) {
        /* Complex SIMD loop with reduction and data-dependent condition */
        #pragma omp simd reduction(+:sum) linear(i:1) safelen(16)
        for (int i = 0; i < SIZE; i++) {
            /* Data-dependent condition inside SIMD loop */
            if (data[i] > 250.0f) {
                /* This break creates complexity for SIMT transformation */
                if (i > SIZE/2) continue;
            }
            
            /* Mixed data type operations */
            float temp = data[i] * 2.0f + 1.0f;
            data[i] = temp;
            sum += temp;
            
            /* Cross-array dependency */
            dbl_data[i] += (double)temp * 0.5;
        }
        
        /* Nested loops with inner SIMD - often triggers SIMT transformation */
        #pragma omp parallel for simd collapse(2) reduction(+:dbl_sum)
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                int idx = i * 32 + j;
                if (idx < SIZE) {
                    dbl_data[idx] = dbl_data[idx] * 1.1 + 0.1;
                    dbl_sum += dbl_data[idx];
                }
            }
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < SIZE; i++) {
            float temp = data[i] * 2.0f + 1.0f;
            data[i] = temp;
            sum += temp;
            dbl_data[i] += (double)temp * 0.5;
        }
        
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                int idx = i * 32 + j;
                if (idx < SIZE) {
                    dbl_data[idx] = dbl_data[idx] * 1.1 + 0.1;
                    dbl_sum += dbl_data[idx];
                }
            }
        }
    }
    
    /* Unconditional SIMD loop with non-unit stride - always present */
    /* This ensures SIMD constructs are parsed regardless of runtime condition */
    #pragma omp simd safelen(8) aligned(strided_data:16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-unit stride access pattern */
        strided_data[i * 2] = strided_data[i * 2] * 3.0f + strided_data[i * 2 + 1];
    }
    
    /* Indirect indexing SIMD loop - complex memory pattern */
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        int idx = indices[i];
        if (idx >= 0 && idx < SIZE) {
            data[idx] = data[idx] * 0.9f + data[i] * 0.1f;
        }
    }
    
    /* GPU offloading section - may trigger full SIMT transformation */
    if (use_offload) {
        /* Target offloading with SIMD - high likelihood for SIMT path */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE]) map(to: indices[0:SIZE]) \
            reduction(+:sum) num_teams(4) thread_limit(128)
        for (int i = 0; i < SIZE; i++) {
            /* Complex operation with conditional */
            float val = data[i];
            if (val > 100.0f) {
                val = val * 0.5f;
            } else {
                val = val * 2.0f;
            }
            data[i] = val;
            sum += val;
            
            /* Additional operation to increase complexity */
            if (i % 8 == 0) {
                data[i] += 1.0f;
            }
        }
        
        /* Nested offload with SIMD */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: dbl_data[0:SIZE/2]) \
            collapse(2) num_teams(2)
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 32; j++) {
                int idx = i * 32 + j;
                if (idx < SIZE/2) {
                    dbl_data[idx] = dbl_data[idx] * 2.0 - 1.0;
                }
            }
        }
    }
    
    /* Use vector types with OpenMP SIMD */
    v4sf vec_data[4];
    for (int i = 0; i < 4; i++) {
        vec_data[i] = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    }
    
    #pragma omp simd
    for (int i = 0; i < 4; i++) {
        vec_data[i] = vec_data[i] * 2.0f + (v4sf){0.5f, 0.5f, 0.5f, 0.5f};
    }
    
    /* Prevent dead code elimination */
    printf("Results: sum = %f, dbl_sum = %lf\n", sum, dbl_sum);
    printf("Sample data[0] = %f, data[%d] = %f\n", 
           data[0], SIZE-1, data[SIZE-1]);
    printf("Sample strided_data[10] = %f\n", strided_data[10]);
    printf("Vector result: %f %f %f %f\n", 
           vec_data[0][0], vec_data[0][1], vec_data[0][2], vec_data[0][3]);
    
    return 0;
}
