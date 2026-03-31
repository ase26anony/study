/* test_omp_simt.c - Program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define STRIDE_SIZE 512

/* Vector type for mixed data type operations */
typedef float v4sf __attribute__((vector_size(16)));

/* Function with conditional SIMD execution */
void conditional_simd_loop(float* data, float* output, int size, int use_simd, float* sum) {
    float local_sum = 0.0f;
    
    if (use_simd) {
        /* This conditional SIMD execution may trigger IFN_GOMP_USE_SIMT */
        #pragma omp target teams distribute parallel for simd \
            map(to:data[0:size]) map(from:output[0:size]) \
            reduction(+:local_sum) if(target:use_simd)
        for (int i = 0; i < size; i++) {
            /* Complex data-dependent computation with potential early exit pattern */
            float val = data[i];
            if (val > 1000.0f) {
                /* This break creates data dependency that SIMT must handle */
                output[i] = 0.0f;
                local_sum += 0.0f;
                continue;
            }
            
            /* Mixed computation with transcendental function */
            output[i] = val * 2.0f + sinf(val * 0.01f) + 1.0f;
            
            /* Reduction with conditional */
            if (output[i] > 50.0f) {
                local_sum += output[i];
            } else {
                local_sum += val;
            }
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < size; i++) {
            float val = data[i];
            output[i] = val * 2.0f + 1.0f;
            local_sum += output[i];
        }
    }
    
    *sum = local_sum;
}

/* Function with nested loops and SIMD on inner loop */
void nested_simd_loop(float* data, int size) {
    int block_size = 32;
    
    #pragma omp parallel for
    for (int block = 0; block < size; block += block_size) {
        int end = (block + block_size < size) ? block + block_size : size;
        
        /* Inner loop with SIMD and safelen clause */
        #pragma omp simd safelen(8) linear(i:1) aligned(data:16)
        for (int i = block; i < end; i++) {
            /* Non-contiguous access pattern */
            if (i % 2 == 0) {
                data[i] = data[i] * 3.0f - data[(i + 1) % size] * 0.5f;
            } else {
                data[i] = sqrtf(fabsf(data[i])) + data[(i - 1 + size) % size];
            }
        }
    }
}

/* Function with explicit vector types and OpenMP SIMD */
void vector_type_simd(v4sf* vec_data, int vec_count) {
    v4sf scale = {1.5f, 2.0f, 2.5f, 3.0f};
    
    #pragma omp simd
    for (int i = 0; i < vec_count; i++) {
        /* Vector operations that may require lane management */
        vec_data[i] = vec_data[i] * scale + (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
        
        /* Conditional within SIMD loop */
        for (int lane = 0; lane < 4; lane++) {
            if (vec_data[i][lane] > 10.0f) {
                vec_data[i][lane] = 10.0f;
            }
        }
    }
}

/* Function with stride access pattern */
void stride_access_simd(float* data, int size) {
    /* This unconditional SIMD loop with stride may hit the SIMT path */
    #pragma omp simd safelen(4)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-unit stride access */
        data[i * 2] = data[i * 2] * 2.5f + sinf(i * 0.1f);
        
        /* Additional computation with indirect pattern */
        int idx = (i * 3) % size;
        if (idx < size) {
            data[idx] = data[idx] * 0.8f;
        }
    }
}

/* Function with reduction and complex data dependency */
float reduction_with_dependency(float* data, int size, int use_simt) {
    float sum = 0.0f;
    float max_val = -INFINITY;
    
    /* SIMD loop with reduction and data-dependent if */
    #pragma omp simd reduction(+:sum) reduction(max:max_val) \
        if(use_simt) linear(i:1)
    for (int i = 0; i < size; i++) {
        float val = data[i];
        
        /* Complex conditional that creates control flow divergence */
        if (val > 0.0f) {
            sum += val * val;
            max_val = (val > max_val) ? val : max_val;
        } else if (val < -10.0f) {
            sum += fabsf(val);
            max_val = (fabsf(val) > max_val) ? fabsf(val) : max_val;
        } else {
            sum += 1.0f;
        }
        
        /* Early exit simulation */
        if (sum > 1000000.0f) {
            /* This creates interesting control flow for SIMT */
            sum = 1000000.0f;
        }
    }
    
    return sum + max_val;
}

int main(int argc, char** argv) {
    /* Runtime condition for conditional SIMD execution */
    int use_simd = (argc > 1);
    int use_simt = (argc > 2);
    
    /* Allocate and initialize arrays */
    float* data = (float*)aligned_alloc(16, SIZE * sizeof(float));
    float* output = (float*)aligned_alloc(16, SIZE * sizeof(float));
    v4sf* vec_data = (v4sf*)aligned_alloc(16, (SIZE/4) * sizeof(v4sf));
    
    if (!data || !output || !vec_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)(i % 100) * 1.5f;
        output[i] = 0.0f;
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        for (int j = 0; j < 4; j++) {
            vec_data[i][j] = (float)((i * 4 + j) % 50) * 0.7f;
        }
    }
    
    float sum1 = 0.0f, sum2 = 0.0f;
    
    /* 1. Conditional SIMD execution with offloading */
    conditional_simd_loop(data, output, SIZE, use_simd, &sum1);
    
    /* 2. Nested loops with SIMD on inner loop */
    nested_simd_loop(data, SIZE);
    
    /* 3. Unconditional SIMD with stride (always present) */
    stride_access_simd(data, SIZE);
    
    /* 4. Vector type operations with SIMD */
    vector_type_simd(vec_data, SIZE/4);
    
    /* 5. Reduction with complex dependency */
    sum2 = reduction_with_dependency(output, SIZE, use_simt);
    
    /* Aggregate results to prevent dead code elimination */
    float total = sum1 + sum2;
    for (int i = 0; i < 10; i++) {
        total += data[i] + output[i];
    }
    
    printf("Result: total = %f, sum1 = %f, sum2 = %f\n", 
           total, sum1, sum2);
    printf("Sample values: data[0]=%f, output[0]=%f\n", 
           data[0], output[0]);
    
    /* Cleanup */
    free(data);
    free(output);
    free(vec_data);
    
    return 0;
}
