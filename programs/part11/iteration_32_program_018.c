/* test_omp_simt.c - Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Mixed data types and vector operations */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

#define SIZE 1024
#define STRIDE 2

int main(int argc, char **argv) {
    /* Use command-line argument to control runtime SIMD condition */
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    /* Declare arrays with mixed data types */
    float data_f[SIZE];
    double data_d[SIZE];
    int indices[SIZE];
    v4sf vec_data[SIZE/4];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data_f[i] = (float)i * 0.5f;
        data_d[i] = (double)i * 0.25;
        indices[i] = (i * 3) % SIZE;
        if (i % 4 == 0) {
            vec_data[i/4] = (v4sf){i*1.0f, i*1.1f, i*1.2f, i*1.3f};
        }
    }
    
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
    /* Conditional SIMD execution based on runtime flag */
    if (use_simd) {
        /* This may trigger the conditional wrapper with IFN_GOMP_USE_SIMT */
        if (use_offload) {
            /* Target offloading directive - often uses SIMT transformation */
            #pragma omp target teams distribute parallel for simd \
                map(tofrom: data_f[0:SIZE], data_d[0:SIZE]) \
                map(to: indices[0:SIZE]) \
                reduction(+:sum_f, sum_d)
            for (int i = 0; i < SIZE; i++) {
                /* Complex data-dependent condition inside SIMD loop */
                if (data_f[i] > 100.0f) {
                    /* Early exit - creates control flow divergence */
                    data_f[i] = 100.0f;
                }
                
                /* Non-contiguous memory access with stride */
                if (i % STRIDE == 0) {
                    data_f[i] = data_f[i] * 2.0f + 1.0f;
                }
                
                /* Indirect indexing */
                data_d[i] = data_d[indices[i]] * 1.5;
                
                /* Reduction operations */
                sum_f += data_f[i];
                sum_d += data_d[i];
            }
        } else {
            /* Regular SIMD with complex clauses */
            #pragma omp simd safelen(16) linear(i:1) aligned(data_f:16) \
                reduction(+:sum_f, sum_d)
            for (int i = 0; i < SIZE; i++) {
                /* Data-dependent condition */
                if (data_f[i] > 50.0f && i % 3 == 0) {
                    data_f[i] = data_f[i] * 0.5f;
                }
                
                /* Mixed data type operations */
                data_f[i] = (float)(data_d[i] * 2.0) + data_f[i];
                sum_f += data_f[i];
                sum_d += data_d[i];
            }
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < SIZE; i++) {
            data_f[i] = data_f[i] * 2.0f + 1.0f;
            data_d[i] = data_d[i] * 1.5;
            sum_f += data_f[i];
            sum_d += data_d[i];
        }
    }
    
    /* Unconditionally present SIMD loop with non-unit stride */
    /* This ensures SIMD constructs are always parsed */
    #pragma omp simd safelen(8)
    for (int i = 0; i < SIZE/STRIDE; i++) {
        /* Non-unit stride access pattern */
        data_f[i * STRIDE] = data_f[i * STRIDE] * 3.0f;
        
        /* Additional complexity with vector types */
        if (i % 4 == 0 && i < SIZE/4) {
            vec_data[i/4] = vec_data[i/4] * 2.0f;
        }
    }
    
    /* Nested loops where inner loop is SIMD */
    for (int block = 0; block < 4; block++) {
        int start = block * (SIZE/4);
        int end = start + (SIZE/4);
        
        #pragma omp simd reduction(+:sum_f)
        for (int i = start; i < end; i++) {
            /* Conditional with potential for lane masking */
            if (data_f[i] < 200.0f) {
                data_f[i] = data_f[i] + (float)i;
            } else {
                data_f[i] = data_f[i] - (float)i;
            }
            sum_f += data_f[i];
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results:\n");
    printf("  sum_f = %f\n", sum_f);
    printf("  sum_d = %lf\n", sum_d);
    printf("  data_f[0] = %f\n", data_f[0]);
    printf("  data_f[100] = %f\n", data_f[100]);
    printf("  data_f[500] = %f\n", data_f[500]);
    printf("  SIMD was %s\n", use_simd ? "enabled" : "disabled");
    printf("  Offload was %s\n", use_offload ? "enabled" : "disabled");
    
    return 0;
}
