/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define STRIDE_SIZE 512

/* Mixed data types for complex access patterns */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Function with conditional SIMD execution */
void process_array_conditional(int use_simd, float* data, float* data2, int* indices, int n, float* sum_out) {
    float sum = 0.0f;
    
    if (use_simd) {
        /* This conditional SIMD block may trigger IFN_GOMP_USE_SIMT */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:n], data2[0:n]) \
            map(to: indices[0:n]) \
            reduction(+:sum) \
            if(target: use_simd)
        for (int i = 0; i < n; i++) {
            /* Complex data-dependent computation with conditional break */
            float val = data[i] * 2.0f + 1.0f;
            data[i] = val;
            
            /* Reduction with mixed operations */
            sum += val * 0.5f;
            
            /* Data-dependent condition that might affect SIMT execution */
            if (val > 1000.0f && i > n/2) {
                /* Early exit - complicates SIMT execution */
                data[i] = 0.0f;
            }
            
            /* Non-contiguous access pattern */
            if (i % 2 == 0) {
                data2[indices[i] % n] += val;
            }
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < n; i++) {
            float val = data[i] * 2.0f + 1.0f;
            data[i] = val;
            sum += val * 0.5f;
            if (i % 2 == 0) {
                data2[indices[i] % n] += val;
            }
        }
    }
    
    *sum_out = sum;
}

/* Function with unconditional SIMD containing complex patterns */
void process_array_unconditional(float* data, int n, float* partial_sums) {
    /* Nested loops with SIMD on inner loop */
    #pragma omp parallel for
    for (int block = 0; block < 4; block++) {
        int start = block * (n / 4);
        int end = (block + 1) * (n / 4);
        float block_sum = 0.0f;
        
        /* Inner SIMD loop with safelen clause and non-unit stride */
        #pragma omp simd reduction(+:block_sum) safelen(8) linear(i:1) aligned(data:32)
        for (int i = start; i < end; i += 1) {
            /* Mixed data type operations */
            float temp = data[i];
            int int_temp = (int)(temp * 100.0f);
            data[i] = temp * 3.0f + sinf((float)int_temp * 0.01f);
            block_sum += data[i];
            
            /* Vector type usage within SIMD loop */
            v4sf vec = {data[i], data[i]*0.5f, data[i]*0.25f, data[i]*0.125f};
            float vec_sum = vec[0] + vec[1] + vec[2] + vec[3];
            data[i] += vec_sum * 0.1f;
        }
        
        partial_sums[block] = block_sum;
    }
    
    /* Another SIMD loop with stride access pattern */
    #pragma omp simd
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-unit stride access - important for SIMT handling */
        data[i * 2] = data[i * 2] * 2.5f + 1.0f;
    }
}

/* Function using explicit vector types with OpenMP */
void vector_operations(v4sf* vec_data, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Vector operations that may require lane management */
        v4sf a = vec_data[i];
        v4sf b = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf c = a + b;
        v4sf d = c * a;
        vec_data[i] = d;
        
        /* Conditional within vectorized loop */
        if (i % 8 == 0) {
            vec_data[i] = vec_data[i] * 0.5f;
        }
    }
}

int main(int argc, char** argv) {
    /* Use command line argument to control SIMD execution path */
    int use_simd = argc > 1 ? atoi(argv[1]) : 0;
    
    /* Allocate and initialize arrays with different patterns */
    float* data = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float* data2 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    float partial_sums[4];
    v4sf* vec_data = (v4sf*)aligned_alloc(16, (SIZE/4) * sizeof(v4sf));
    
    /* Initialize with varying patterns */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.1f;
        data2[i] = (float)(SIZE - i) * 0.05f;
        indices[i] = (i * 13) % SIZE;  /* Pseudo-random pattern */
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        vec_data[i] = (v4sf){i*0.1f, i*0.2f, i*0.3f, i*0.4f};
    }
    
    float sum_result = 0.0f;
    
    /* Call conditional SIMD function - may trigger the uncovered block */
    process_array_conditional(use_simd, data, data2, indices, SIZE, &sum_result);
    
    /* Always execute unconditional SIMD patterns */
    process_array_unconditional(data, SIZE, partial_sums);
    
    /* Execute vector operations */
    vector_operations(vec_data, SIZE/4);
    
    /* Aggregate and print results to prevent optimization */
    float total_sum = sum_result;
    for (int i = 0; i < 4; i++) {
        total_sum += partial_sums[i];
    }
    
    /* Sample output to verify computation */
    printf("Total sum: %f\n", total_sum);
    printf("Sample data[0]: %f, data[100]: %f, data2[50]: %f\n", 
           data[0], data[100], data2[50]);
    printf("Vector sample: %f, %f, %f, %f\n",
           vec_data[0][0], vec_data[0][1], vec_data[0][2], vec_data[0][3]);
    
    /* Cleanup */
    free(data);
    free(data2);
    free(indices);
    free(vec_data);
    
    return 0;
}
