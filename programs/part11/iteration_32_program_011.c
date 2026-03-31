/* test_omp_simt.c - Program to trigger SIMT transformation in GCC's omp-low.cc */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Define a vector type to encourage SIMD/SIMT handling */
typedef float v4sf __attribute__((vector_size(16)));

/* Function with complex data-dependent condition */
void process_array_with_condition(float *data, int size, float threshold, int use_simd) {
    float sum = 0.0f;
    
    /* Conditional SIMD execution - may trigger IFN_GOMP_USE_SIMT */
    if (use_simd) {
        /* This complex loop with reduction and condition may trigger SIMT lowering */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:size]) reduction(+:sum) if(target: use_simd)
        for (int i = 0; i < size; i++) {
            /* Data-dependent operation with potential early exit pattern */
            data[i] = data[i] * 2.0f + 1.0f;
            
            /* Reduction operation */
            sum += data[i];
            
            /* Complex condition that might require lane masking in SIMT */
            if (data[i] > threshold && i % 8 == 0) {
                /* This creates control flow divergence */
                data[i] = sqrtf(data[i]);
            }
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < size; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
            if (data[i] > threshold && i % 8 == 0) {
                data[i] = sqrtf(data[i]);
            }
        }
    }
    
    printf("Conditional SIMD sum: %f\n", sum);
}

/* Function with mixed data types and non-contiguous access */
void process_mixed_types(double *dbl_arr, float *flt_arr, int *idx_arr, int size) {
    /* SIMD loop with safelen clause and non-unit stride */
    #pragma omp simd safelen(8) aligned(dbl_arr, flt_arr: 32) linear(i:1)
    for (int i = 0; i < size/2; i++) {
        /* Non-contiguous access pattern */
        int idx = idx_arr[i];
        if (idx < size) {
            /* Mixed type operations */
            flt_arr[idx * 2] = (float)dbl_arr[i] * 3.0f;
            
            /* Complex expression with type conversion */
            dbl_arr[i] = (double)flt_arr[idx * 2] / 2.0 + sin((double)i);
        }
    }
}

/* Nested loops with SIMD on inner loop */
void nested_simd_processing(float *matrix, int rows, int cols) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            /* Inner SIMD loop if compiler chooses to apply it */
            #pragma omp simd reduction(+:matrix[i*cols + j])
            for (int k = 0; k < 4; k++) {
                matrix[i*cols + j] += (float)(i + j + k) * 0.5f;
            }
        }
    }
}

/* Function using explicit vector types */
void vector_type_processing(v4sf *vec_data, int vec_count) {
    /* SIMD loop with explicit vector types */
    #pragma omp simd
    for (int i = 0; i < vec_count; i++) {
        /* Vector operations that may require SIMT lane management */
        v4sf temp = vec_data[i] * (v4sf){2.0f, 1.5f, 1.0f, 0.5f};
        vec_data[i] = temp + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    }
}

int main(int argc, char *argv[]) {
    const int SIZE = 1024;
    const int VEC_SIZE = 256;
    
    /* Use command line argument to control SIMD execution */
    int use_simd = (argc > 1) ? atoi(argv[1]) : 0;
    float threshold = 100.0f;
    
    /* Allocate and initialize arrays */
    float *data = (float*)aligned_alloc(32, SIZE * sizeof(float));
    double *dbl_arr = (double*)aligned_alloc(32, SIZE * sizeof(double));
    float *flt_arr = (float*)aligned_alloc(32, SIZE * sizeof(float));
    int *idx_arr = (int*)malloc(SIZE/2 * sizeof(int));
    v4sf *vec_data = (v4sf*)aligned_alloc(16, VEC_SIZE * sizeof(v4sf));
    float *matrix = (float*)malloc(64 * 64 * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i;
        dbl_arr[i] = (double)i * 0.5;
        flt_arr[i] = (float)i * 1.5f;
    }
    
    for (int i = 0; i < SIZE/2; i++) {
        idx_arr[i] = (i * 3) % SIZE;
    }
    
    for (int i = 0; i < VEC_SIZE; i++) {
        vec_data[i] = (v4sf){(float)i, (float)i+1, (float)i+2, (float)i+3};
    }
    
    for (int i = 0; i < 64*64; i++) {
        matrix[i] = (float)i * 0.1f;
    }
    
    printf("Starting processing with use_simd=%d\n", use_simd);
    
    /* Call functions that may trigger SIMT transformation */
    process_array_with_condition(data, SIZE, threshold, use_simd);
    
    /* Always include some SIMD constructs */
    process_mixed_types(dbl_arr, flt_arr, idx_arr, SIZE);
    
    /* Nested processing */
    nested_simd_processing(matrix, 64, 64);
    
    /* Vector type processing */
    vector_type_processing(vec_data, VEC_SIZE);
    
    /* Print some results to prevent dead code elimination */
    printf("Sample results:\n");
    printf("data[0]=%f, data[100]=%f, data[1000]=%f\n", 
           data[0], data[100], data[1000]);
    printf("dbl_arr[50]=%f, flt_arr[200]=%f\n", 
           dbl_arr[50], flt_arr[200]);
    printf("matrix[1000]=%f, vec_data[10][0]=%f\n", 
           matrix[1000], vec_data[10][0]);
    
    /* Cleanup */
    free(data);
    free(dbl_arr);
    free(flt_arr);
    free(idx_arr);
    free(vec_data);
    free(matrix);
    
    return 0;
}
