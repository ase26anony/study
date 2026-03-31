/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define STRIDE_SIZE 512

/* Vector type for mixed data operations */
typedef float v4sf __attribute__((vector_size(16)));

/* Function with conditional SIMD execution */
void process_array_conditional(int use_simd, float* data, float* data2, 
                               int* indices, float* result, int n) {
    float sum = 0.0f;
    
    /* Conditional SIMD execution - may trigger IFN_GOMP_USE_SIMT */
    if (use_simd) {
        /* Target offloading with SIMD - likely to trigger SIMT lowering */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:n], data2[0:n]) \
            map(to: indices[0:n]) \
            map(tofrom: sum) \
            reduction(+:sum) \
            safelen(32)
        for (int i = 0; i < n; i++) {
            /* Complex data-dependent computation with conditional */
            float val = data[i] * 2.0f + data2[i];
            if (val > 100.0f) {
                val = 100.0f;  /* Data-dependent condition */
            }
            data[i] = sinf(val) * cosf(val);
            sum += data[i];
            
            /* Non-contiguous access pattern */
            if (i % 2 == 0) {
                data2[indices[i]] = data[i] * 0.5f;
            }
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < n; i++) {
            float val = data[i] * 2.0f + data2[i];
            if (val > 100.0f) {
                val = 100.0f;
            }
            data[i] = sinf(val) * cosf(val);
            sum += data[i];
            if (i % 2 == 0) {
                data2[indices[i]] = data[i] * 0.5f;
            }
        }
    }
    
    *result = sum;
}

/* Function with unconditional SIMD and mixed data types */
void process_with_mixed_types(float* data, double* ddata, int n) {
    /* SIMD with safelen clause and non-unit stride */
    #pragma omp simd safelen(16) aligned(data:16) linear(i:1)
    for (int i = 0; i < n/2; i++) {
        /* Non-unit stride access */
        data[i*2] = data[i*2] * 3.0f + (float)ddata[i];
        
        /* Data-dependent operation that might require lane masking */
        if (data[i*2] > 50.0f) {
            data[i*2] = 50.0f;
        }
    }
    
    /* Another SIMD loop with reduction */
    double sum = 0.0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += (double)data[i];
        /* Mixed precision computation */
        ddata[i] = (double)data[i] * 1.5;
    }
}

/* Nested loops with SIMD on inner loop */
void nested_simd_operations(float* A, float* B, float* C, int m, int n) {
    #pragma omp for simd collapse(2)
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            int idx = i * n + j;
            C[idx] = A[idx] * B[idx] + (float)(i + j);
            
            /* Conditional break simulation - complex for SIMT */
            if (C[idx] > 1000.0f && j > n/2) {
                C[idx] = 1000.0f;
            }
        }
    }
}

int main(int argc, char** argv) {
    /* Use command line argument to control SIMD execution */
    int use_simd = (argc > 1) ? atoi(argv[1]) : 1;
    
    /* Allocate and initialize arrays */
    float* data = (float*)aligned_alloc(16, SIZE * sizeof(float));
    float* data2 = (float*)aligned_alloc(16, SIZE * sizeof(float));
    double* ddata = (double*)malloc(SIZE * sizeof(double));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    float* A = (float*)malloc(SIZE * SIZE/16 * sizeof(float));
    float* B = (float*)malloc(SIZE * SIZE/16 * sizeof(float));
    float* C = (float*)malloc(SIZE * SIZE/16 * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)(i % 100);
        data2[i] = (float)((i + 50) % 100);
        ddata[i] = (double)(i % 50);
        indices[i] = (i * 3) % SIZE;
    }
    
    for (int i = 0; i < SIZE * SIZE/16; i++) {
        A[i] = (float)(i % 255);
        B[i] = (float)((i + 127) % 255);
    }
    
    float result = 0.0f;
    
    /* Call function with conditional SIMD execution */
    process_array_conditional(use_simd, data, data2, indices, &result, SIZE);
    
    /* Process with mixed types (always uses SIMD) */
    process_with_mixed_types(data, ddata, SIZE);
    
    /* Nested SIMD operations */
    nested_simd_operations(A, B, C, SIZE/16, SIZE/16);
    
    /* Aggregate and print results to prevent dead code elimination */
    printf("Result sum: %f\n", result);
    printf("Sample data[0]: %f, data[100]: %f, data[500]: %f\n", 
           data[0], data[100], data[500]);
    printf("Sample ddata[0]: %f, ddata[100]: %f\n", ddata[0], ddata[100]);
    printf("Sample C[1000]: %f\n", C[1000]);
    
    /* Cleanup */
    free(data);
    free(data2);
    free(ddata);
    free(indices);
    free(A);
    free(B);
    free(C);
    
    return 0;
}
