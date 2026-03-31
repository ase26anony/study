/* test_omp_simt.c
 * This program is designed to trigger the SIMT transformation path in GCC's omp-low.cc,
 * specifically targeting the uncovered block that builds a conditional wrapper with
 * IFN_GOMP_USE_SIMT. It uses various OpenMP SIMD constructs, offloading directives,
 * conditional execution, and complex memory access patterns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define STRIDE 2

/* Explicit vector type to encourage vectorization analysis */
typedef float v4sf __attribute__((vector_size(16)));

int main(int argc, char *argv[])
{
    /* Runtime condition to control SIMD path execution */
    int use_simd = argc > 1;  /* Enable SIMD if any argument is given */
    
    /* Arrays with mixed data types and non-contiguous access patterns */
    float data[N];
    double double_data[N];
    int indices[N];
    v4sf vector_data[N/4];
    
    /* Reduction variable */
    float sum = 0.0f;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        data[i] = (float)i * 0.1f;
        double_data[i] = (double)i * 0.2;
        indices[i] = (i * 7) % N;  /* Non-linear pattern for indirect access */
    }
    
    for (int i = 0; i < N/4; i++) {
        vector_data[i] = (v4sf){i*0.1f, i*0.2f, i*0.3f, i*0.4f};
    }
    
    /* 1. Conditional SIMD execution with target offloading */
    if (use_simd) {
        /* This may trigger the conditional SIMT wrapper generation */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:N]) map(to: indices[0:N]) \
            reduction(+:sum) if(target: use_simd)
        for (int i = 0; i < N; i++) {
            /* Complex data-dependent operation with conditional break */
            float val = data[i] * 2.0f + 1.0f;
            if (val > 1000.0f) {
                /* Data-dependent exit - encourages SIMT masking */
                val = 1000.0f;
            }
            data[i] = val;
            sum += val;
            
            /* Indirect memory access */
            data[indices[i]] += 0.01f;
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < N; i++) {
            float val = data[i] * 2.0f + 1.0f;
            if (val > 1000.0f) {
                val = 1000.0f;
            }
            data[i] = val;
            sum += val;
            data[indices[i]] += 0.01f;
        }
    }
    
    /* 2. Unconditional SIMD loop with safelen and non-unit stride */
    /* This ensures SIMD constructs are always parsed */
    #pragma omp simd safelen(8) aligned(data: 16)
    for (int i = 0; i < N/STRIDE; i++) {
        /* Non-contiguous memory access pattern */
        data[i * STRIDE] = data[i * STRIDE] * 3.0f;
    }
    
    /* 3. Nested loops with SIMD on inner loop */
    /* Mixed data types in computation */
    #pragma omp parallel for
    for (int i = 0; i < N-1; i++) {
        #pragma omp simd linear(j:1)
        for (int j = 0; j < 4; j++) {
            double_data[i + j] = (double)data[i] * 0.5 + double_data[i + j];
        }
    }
    
    /* 4. SIMD loop with explicit vector types */
    #pragma omp simd
    for (int i = 0; i < N/4; i++) {
        vector_data[i] = vector_data[i] * 2.0f + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    }
    
    /* 5. SIMD reduction with conditional inside loop */
    float max_val = 0.0f;
    #pragma omp simd reduction(max:max_val)
    for (int i = 0; i < N; i++) {
        if (data[i] > max_val) {
            max_val = data[i];
        }
    }
    
    /* Prevent dead code elimination */
    printf("Results:\n");
    printf("  sum = %f\n", sum);
    printf("  max_val = %f\n", max_val);
    printf("  data[0] = %f, data[100] = %f, data[500] = %f\n", 
           data[0], data[100], data[500]);
    printf("  SIMD path %s\n", use_simd ? "enabled" : "disabled");
    
    return 0;
}
