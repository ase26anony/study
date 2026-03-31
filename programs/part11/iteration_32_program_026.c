/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define STRIDE_SIZE 512

/* Vector type for mixed data type operations */
typedef float v4sf __attribute__((vector_size(16)));

/* Function with conditional SIMD execution */
void process_data_conditional(int use_simd, float* data, float* data2, int* indices, float* result) {
    float sum = 0.0f;
    
    if (use_simd) {
        /* This conditional SIMD block may trigger IFN_GOMP_USE_SIMT */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE], data2[0:SIZE]) \
            map(to: indices[0:SIZE]) \
            reduction(+:sum) \
            if(target: use_simd)
        for (int i = 0; i < SIZE; i++) {
            /* Complex data-dependent computation with conditional break */
            float val = data[i] * 2.0f + 1.0f;
            
            /* Data-dependent condition that might affect SIMT execution */
            if (val > 1000.0f && i > SIZE/2) {
                /* Early exit - creates complex control flow for SIMT */
                val = 1000.0f;
            }
            
            /* Non-contiguous memory access pattern */
            data2[indices[i]] = val * 0.5f;
            
            /* Reduction operation */
            sum += val;
            
            /* Mixed data type operation */
            data[i] = (float)((double)val * 0.25);
        }
        *result = sum;
    } else {
        /* Sequential fallback */
        for (int i = 0; i < SIZE; i++) {
            float val = data[i] * 2.0f + 1.0f;
            if (val > 1000.0f && i > SIZE/2) {
                val = 1000.0f;
            }
            data2[indices[i]] = val * 0.5f;
            sum += val;
            data[i] = (float)((double)val * 0.25);
        }
        *result = sum;
    }
}

/* Function with unconditional SIMD containing safelen clause */
void process_data_unconditional(float* data, float* output) {
    /* Unconditional SIMD with safelen clause and non-unit stride */
    #pragma omp simd safelen(8) aligned(data, output: 16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-unit stride access pattern */
        output[i] = data[i * 2] * 3.0f + sinf((float)i * 0.01f);
    }
}

/* Nested loop with inner SIMD */
void nested_simd_loop(float* matrix, int rows, int cols) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            /* Inner SIMD loop with linear clause */
            #pragma omp simd linear(j:1)
            for (int k = 0; k < 8; k++) {
                int idx = (i * cols + j) * 8 + k;
                matrix[idx] = matrix[idx] * (float)(i + j + k) / 100.0f;
            }
        }
    }
}

/* Function using explicit vector types */
void vector_type_operation(v4sf* vec_data, int count) {
    #pragma omp simd
    for (int i = 0; i < count; i++) {
        /* Vector operations that may require special SIMT handling */
        v4sf temp = vec_data[i] * (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
        vec_data[i] = temp + (v4sf){0.5f, 0.5f, 0.5f, 0.5f};
    }
}

int main(int argc, char** argv) {
    /* Runtime condition to control SIMD execution path */
    int use_simd = (argc > 1) ? atoi(argv[1]) : 0;
    
    /* Allocate and initialize arrays */
    float* data = (float*)aligned_alloc(16, SIZE * sizeof(float));
    float* data2 = (float*)aligned_alloc(16, SIZE * sizeof(float));
    float* output = (float*)aligned_alloc(16, STRIDE_SIZE * sizeof(float));
    float* matrix = (float*)malloc(32 * 32 * 8 * sizeof(float));
    v4sf* vec_data = (v4sf*)aligned_alloc(16, 64 * sizeof(v4sf));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.1f;
        data2[i] = (float)(SIZE - i) * 0.05f;
        indices[i] = (i * 7) % SIZE;  /* Create non-linear access pattern */
    }
    
    for (int i = 0; i < STRIDE_SIZE; i++) {
        output[i] = 0.0f;
    }
    
    for (int i = 0; i < 32 * 32 * 8; i++) {
        matrix[i] = (float)(i % 100) * 0.01f;
    }
    
    for (int i = 0; i < 64; i++) {
        vec_data[i] = (v4sf){ (float)i, (float)(i+1), (float)(i+2), (float)(i+3) };
    }
    
    float result = 0.0f;
    
    /* Call functions that may trigger SIMT transformation */
    process_data_conditional(use_simd, data, data2, indices, &result);
    
    process_data_unconditional(data, output);
    
    nested_simd_loop(matrix, 32, 32);
    
    vector_type_operation(vec_data, 64);
    
    /* Aggregate and print results to prevent dead code elimination */
    printf("Result sum: %f\n", result);
    printf("Sample data[0]: %f, data[100]: %f\n", data[0], data[100]);
    printf("Sample output[10]: %f\n", output[10]);
    printf("Sample matrix[100]: %f\n", matrix[100]);
    printf("Sample vec_data[0]: %f %f %f %f\n", 
           vec_data[0][0], vec_data[0][1], vec_data[0][2], vec_data[0][3]);
    
    /* Cleanup */
    free(data);
    free(data2);
    free(output);
    free(matrix);
    free(vec_data);
    free(indices);
    
    return 0;
}
