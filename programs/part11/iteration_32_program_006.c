/* test_omp_simt.c - Program to trigger SIMT transformation in omp-low.cc */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define STRIDE_SIZE 512

/* Mixed data types to test SIMT handling */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Function with conditional SIMD execution */
void conditional_simd_computation(float* data, int* indices, double* results, 
                                  int size, int use_simd, int argc) {
    double sum = 0.0;
    float threshold = 0.5f;
    
    /* Runtime condition that may trigger SIMT conditional wrapper */
    if (use_simd) {
        /* This block should trigger the conditional SIMT path */
        /* Use target offloading directive with SIMD */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:size]) map(to: indices[0:size]) \
            map(tofrom: sum) reduction(+:sum) if(target: use_simd)
        for (int i = 0; i < size; i++) {
            /* Non-contiguous access pattern */
            float val = data[indices[i]];
            
            /* Data-dependent condition inside SIMD loop */
            if (val > threshold && argc > 2) {
                val = val * 0.5f;
            }
            
            /* Mixed operations */
            data[i] = val * 2.0f + 1.0f;
            
            /* Reduction with conditional */
            sum += (double)data[i];
            
            /* Early exit simulation - complex for SIMT */
            if (data[i] > 1000.0f && i % 32 == 0) {
                /* This creates control flow divergence */
                data[i] = 1000.0f;
            }
        }
        
        /* Store result */
        results[0] = sum;
    } else {
        /* Sequential fallback */
        for (int i = 0; i < size; i++) {
            float val = data[indices[i]];
            if (val > threshold && argc > 2) {
                val = val * 0.5f;
            }
            data[i] = val * 2.0f + 1.0f;
            sum += (double)data[i];
            if (data[i] > 1000.0f && i % 32 == 0) {
                data[i] = 1000.0f;
            }
        }
        results[0] = sum;
    }
    
    /* Always present SIMD loop with complex pattern */
    /* This ensures SIMD constructs are parsed regardless of runtime condition */
    #pragma omp simd safelen(16) linear(i:1) aligned(data:16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-unit stride access */
        data[i * 2] = data[i * 2] * 3.0f + sinf((float)i * 0.01f);
        
        /* Vector type usage within SIMD loop */
        v4sf* vptr = (v4sf*)&data[i * 2];
        if (i % 4 == 0 && i + 3 < STRIDE_SIZE * 2) {
            v4sf temp = *vptr;
            temp = temp + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
            *vptr = temp;
        }
    }
}

/* Nested loop with inner SIMD */
void nested_simd_loop(float* a, float* b, float* c, int n, int m) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            /* Inner SIMD loop with reduction */
            float sum = 0.0f;
            #pragma omp simd reduction(+:sum) safelen(8)
            for (int k = 0; k < 16; k++) {
                /* Complex addressing */
                int idx = (i * m + j) * 16 + k;
                if (idx < n * m * 16) {
                    sum += a[idx] * b[k];
                    /* Conditional store */
                    c[idx] = (sum > 0.0f) ? sum : 0.0f;
                }
            }
            a[i * m + j] = sum;
        }
    }
}

/* Function with SIMD and explicit vector types */
void vector_type_simd(v4sf* vec_data, int* mask, int size) {
    /* SIMD loop with explicit vector operations */
    #pragma omp simd
    for (int i = 0; i < size; i++) {
        /* Lane-dependent operation using mask */
        v4sf temp = vec_data[i];
        
        /* Conditional vector operation */
        if (mask[i] > 0) {
            temp = temp * (v4sf){2.0f, 2.0f, 2.0f, 2.0f};
        } else {
            temp = temp + (v4sf){1.0f, 1.0f, 1.0f, 1.0f};
        }
        
        /* Cross-lane operation simulation */
        float lane_sum = temp[0] + temp[1] + temp[2] + temp[3];
        temp[0] = lane_sum;
        
        vec_data[i] = temp;
    }
}

int main(int argc, char** argv) {
    /* Runtime condition for conditional SIMD execution */
    int use_simd = (argc > 1);
    int use_offload = (argc > 2);
    
    printf("Running with argc=%d, use_simd=%d, use_offload=%d\n", 
           argc, use_simd, use_offload);
    
    /* Allocate and initialize arrays */
    float* data = (float*)aligned_alloc(16, SIZE * sizeof(float));
    float* data2 = (float*)aligned_alloc(16, SIZE * sizeof(float));
    float* data3 = (float*)aligned_alloc(16, SIZE * sizeof(float));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    double* results = (double*)malloc(3 * sizeof(double));
    v4sf* vec_data = (v4sf*)aligned_alloc(16, (SIZE/4) * sizeof(v4sf));
    int* mask = (int*)malloc((SIZE/4) * sizeof(int));
    
    if (!data || !data2 || !data3 || !indices || !results || !vec_data || !mask) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.1f;
        data2[i] = (float)(SIZE - i) * 0.05f;
        data3[i] = sinf((float)i * 0.01f);
        indices[i] = (i * 7) % SIZE;  /* Non-linear indexing */
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        for (int j = 0; j < 4; j++) {
            vec_data[i][j] = (float)(i * 4 + j) * 0.25f;
        }
        mask[i] = (i % 3 == 0) ? 1 : 0;
    }
    
    /* Test 1: Conditional SIMD with offloading */
    conditional_simd_computation(data, indices, results, SIZE, use_simd, argc);
    
    /* Test 2: Nested loops with inner SIMD - always present */
    if (use_offload) {
        #pragma omp target map(tofrom: data2[0:SIZE], data3[0:SIZE]) \
                         if(target: use_offload)
        nested_simd_loop(data2, data3, data, 16, 16);
    } else {
        nested_simd_loop(data2, data3, data, 16, 16);
    }
    
    /* Test 3: Vector type SIMD - unconditional */
    vector_type_simd(vec_data, mask, SIZE/4);
    
    /* Aggregate and print results to prevent dead code elimination */
    double total_sum = results[0];
    for (int i = 0; i < 10; i++) {
        total_sum += data[i] + data2[i] + data3[i];
    }
    
    /* Use vector data */
    for (int i = 0; i < 4 && i < SIZE/4; i++) {
        total_sum += vec_data[i][0] + vec_data[i][1];
    }
    
    printf("Total sum: %f\n", total_sum);
    printf("Sample data[0]=%f, data[100]=%f, data2[50]=%f\n", 
           data[0], data[100], data2[50]);
    
    /* Cleanup */
    free(data);
    free(data2);
    free(data3);
    free(indices);
    free(results);
    free(vec_data);
    free(mask);
    
    return 0;
}
