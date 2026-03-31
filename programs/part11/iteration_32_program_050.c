/* test_omp_simt.c - Program to trigger SIMT transformation in omp-low.cc */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define STRIDE_SIZE 512

/* Mixed data types for complex access patterns */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Function to initialize arrays */
void init_arrays(float *data, int *index, float *data2) {
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.1f;
        data2[i] = (float)(SIZE - i) * 0.05f;
        index[i] = (i * 3) % SIZE;  /* Non-linear indexing pattern */
    }
}

/* Function with conditional SIMD execution */
float process_data_conditional(int use_simd, float *data, int *index, float *data2, int n) {
    float sum = 0.0f;
    float threshold = 50.0f;
    
    /* Runtime condition to potentially use SIMD */
    if (use_simd) {
        /* This block may trigger the conditional SIMT wrapper */
        #pragma omp target teams distribute parallel for simd \
            map(to: data[0:n], index[0:n], data2[0:n]) \
            map(tofrom: sum) reduction(+:sum) \
            if(target: use_simd)
        for (int i = 0; i < n; i++) {
            /* Complex access pattern with indirect indexing */
            int idx = index[i];
            float val = data[idx] * 2.0f + data2[i];
            
            /* Data-dependent condition inside SIMD loop */
            if (val > threshold) {
                /* This break/condition encourages SIMT transformation */
                val = threshold;
            }
            
            data[idx] = val;
            sum += val;
            
            /* Additional computation with mixed types */
            if (i % 2 == 0) {
                data2[i] = (float)((int)val % 10) * 0.5f;
            }
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < n; i++) {
            int idx = index[i];
            float val = data[idx] * 2.0f + data2[i];
            if (val > threshold) val = threshold;
            data[idx] = val;
            sum += val;
            if (i % 2 == 0) {
                data2[i] = (float)((int)val % 10) * 0.5f;
            }
        }
    }
    
    return sum;
}

/* Function with unconditional SIMD construct */
void process_stride_data(float *data, int n) {
    /* Unconditional SIMD loop with safelen clause and non-unit stride */
    #pragma omp simd safelen(8) aligned(data:16)
    for (int i = 0; i < n/2; i++) {
        /* Non-contiguous access pattern */
        data[i * 2] = data[i * 2] * 3.0f + sinf((float)i * 0.01f);
        
        /* Additional conditional to create control flow */
        if (data[i * 2] > 100.0f) {
            data[i * 2] = 100.0f;
        }
    }
}

/* Nested loop with SIMD on inner loop */
void nested_simd_processing(float *data, float *data2, int n) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < n/16; i++) {
        /* Inner loop marked with SIMD - may trigger SIMT transformation */
        #pragma omp simd linear(j:1) reduction(+:data2[i])
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            float temp = data[idx] * 1.5f;
            
            /* Vector-like operation */
            v4sf vec_temp = {temp, temp * 0.5f, temp * 0.25f, temp * 0.125f};
            float vec_sum = vec_temp[0] + vec_temp[1] + vec_temp[2] + vec_temp[3];
            
            data[idx] = vec_sum;
            data2[i] += vec_sum;
            
            /* Early exit condition - encourages SIMT lane masking */
            if (vec_sum > 200.0f && j > 8) {
                data[idx] = 200.0f;
            }
        }
    }
}

/* Main driver function */
int main(int argc, char *argv[]) {
    /* Runtime condition for SIMD execution */
    int use_simd = (argc > 1) ? atoi(argv[1]) : 0;
    
    /* Allocate and initialize arrays */
    float *data = (float*)aligned_alloc(16, SIZE * sizeof(float));
    float *data2 = (float*)aligned_alloc(16, SIZE * sizeof(float));
    int *index = (int*)malloc(SIZE * sizeof(int));
    
    if (!data || !data2 || !index) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(data, index, data2);
    
    printf("Processing with use_simd = %d\n", use_simd);
    
    /* Process data with conditional SIMD execution */
    float sum = process_data_conditional(use_simd, data, index, data2, SIZE);
    
    /* Always execute unconditional SIMD loop */
    process_stride_data(data, SIZE);
    
    /* Execute nested SIMD processing */
    nested_simd_processing(data, data2, SIZE);
    
    /* Aggregate and print results to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Sample values: data[0]=%f, data[100]=%f, data2[500]=%f\n", 
           data[0], data[100], data2[500]);
    
    /* Additional check for vector types */
    v4sf test_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    v4si int_vec = {1, 2, 3, 4};
    test_vec = test_vec * 2.0f;
    
    printf("Vector test: %f %f %f %f\n", 
           test_vec[0], test_vec[1], test_vec[2], test_vec[3]);
    
    /* Cleanup */
    free(data);
    free(data2);
    free(index);
    
    return 0;
}
