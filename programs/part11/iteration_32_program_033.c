/* test_omp_simt.c - Program to trigger SIMT transformation in GCC's omp-low.cc */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define STRIDE_SIZE 512

/* Mixed data types for complex access patterns */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function with conditional SIMD execution */
void process_array_conditional(int use_simd, float* data, double* dbl_data, 
                               int* indices, int n, float* result_sum) {
    float sum = 0.0f;
    
    if (use_simd) {
        /* This conditional SIMD block may trigger IFN_GOMP_USE_SIMT */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:n], dbl_data[0:n]) \
            map(to: indices[0:n]) \
            reduction(+:sum) \
            if(target: use_simd)
        for (int i = 0; i < n; i++) {
            /* Complex data-dependent condition inside SIMD loop */
            if (data[i] > 0.5f) {
                data[i] = sinf(data[i]) * 2.0f;
            } else {
                data[i] = cosf(data[i]) * 0.5f;
            }
            
            /* Mixed data type operations */
            dbl_data[i] = (double)data[i] * 1.5;
            
            /* Reduction with conditional */
            sum += data[i] * (i % 8);
            
            /* Early exit condition - encourages SIMT masking */
            if (data[i] > 100.0f) {
                /* This break creates data-dependent control flow */
                /* continue; */ /* Commented to avoid infinite loops in test */
            }
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < n; i++) {
            if (data[i] > 0.5f) {
                data[i] = sinf(data[i]) * 2.0f;
            } else {
                data[i] = cosf(data[i]) * 0.5f;
            }
            dbl_data[i] = (double)data[i] * 1.5;
            sum += data[i] * (i % 8);
        }
    }
    
    *result_sum = sum;
}

/* Function with nested SIMD loops and non-contiguous access */
void process_strided_access(float* data, int n, float* reduction_var) {
    float local_sum = 0.0f;
    
    /* Outer loop - may be parallelized */
    #pragma omp parallel for reduction(+:local_sum)
    for (int i = 0; i < n/2; i++) {
        /* Inner loop with SIMD directive - may trigger SIMT transformation */
        #pragma omp simd safelen(8) linear(i:1) aligned(data:32) \
                     reduction(+:local_sum)
        for (int j = 0; j < 4; j++) {
            /* Non-unit stride access pattern */
            int idx = i * 2 + j;
            if (idx < n) {
                /* Complex expression with mixed operations */
                data[idx] = data[idx] * (1.0f + 0.1f * j) + 
                           sinf((float)idx * 0.01f);
                
                /* Conditional reduction */
                if (data[idx] > 0.0f) {
                    local_sum += sqrtf(fabsf(data[idx]));
                }
            }
        }
    }
    
    *reduction_var = local_sum;
}

/* Function using explicit vector types with OpenMP SIMD */
void vector_type_operations(v4sf* vec_data, float* scalar_data, int n) {
    /* SIMD loop with explicit vector types */
    #pragma omp simd
    for (int i = 0; i < n/4; i++) {
        /* Load scalar data into vector */
        v4sf vec = {scalar_data[i*4], scalar_data[i*4+1], 
                    scalar_data[i*4+2], scalar_data[i*4+3]};
        
        /* Vector operations */
        v4sf result = vec * (v4sf){1.1f, 1.2f, 1.3f, 1.4f} + 
                     (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
        
        /* Store back - may require gather/scatter in SIMT */
        vec_data[i] = result;
        
        /* Conditional store based on vector comparison */
        if (scalar_data[i*4] > 0.5f) {
            vec_data[i] = result * 2.0f;
        }
    }
}

/* Main driver with runtime condition */
int main(int argc, char** argv) {
    /* Runtime condition for conditional SIMD execution */
    int use_simd = (argc > 1) ? atoi(argv[1]) : 0;
    
    /* Allocate and initialize arrays */
    float* data = (float*)aligned_alloc(32, SIZE * sizeof(float));
    double* dbl_data = (double*)aligned_alloc(32, SIZE * sizeof(double));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    v4sf* vec_data = (v4sf*)aligned_alloc(16, (SIZE/4) * sizeof(v4sf));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i / SIZE;
        dbl_data[i] = (double)i / SIZE;
        indices[i] = (i * 3) % SIZE;  /* Non-linear indexing */
    }
    
    float sum1 = 0.0f, sum2 = 0.0f;
    
    /* Call function with conditional SIMD execution */
    process_array_conditional(use_simd, data, dbl_data, indices, SIZE, &sum1);
    
    /* Always execute strided access function with SIMD */
    process_strided_access(data, SIZE, &sum2);
    
    /* Execute vector type operations */
    vector_type_operations(vec_data, data, SIZE);
    
    /* Additional unconditional SIMD loop with complex pattern */
    float complex_sum = 0.0f;
    #pragma omp simd reduction(+:complex_sum) safelen(16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-contiguous memory access with stride */
        data[i*2] = data[i*2] * 3.0f + sinf((float)i * 0.1f);
        
        /* Data-dependent computation */
        if (i % 4 == 0) {
            complex_sum += data[i*2] * 0.5f;
        } else {
            complex_sum += data[i*2] * 0.25f;
        }
        
        /* Indirect indexing */
        int idx = indices[i % SIZE];
        if (idx < SIZE) {
            dbl_data[idx] += 0.001;
        }
    }
    
    /* Prevent dead code elimination */
    printf("Results: sum1=%.6f, sum2=%.6f, complex_sum=%.6f\n", 
           sum1, sum2, complex_sum);
    printf("Sample data[0]=%.6f, data[100]=%.6f, data[500]=%.6f\n",
           data[0], data[100], data[500]);
    printf("Sample dbl_data[50]=%.6f\n", dbl_data[50]);
    
    /* Cleanup */
    free(data);
    free(dbl_data);
    free(indices);
    free(vec_data);
    
    return 0;
}
